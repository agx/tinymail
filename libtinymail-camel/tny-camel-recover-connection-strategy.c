/* Your copyright here */

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-folder.h>

#include <tny-camel-recover-connection-strategy.h>
#include <tny-camel-account.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyCamelRecoverConnectionStrategyPriv TnyCamelRecoverConnectionStrategyPriv;

struct _TnyCamelRecoverConnectionStrategyPriv
{
	TnyFolder *folder;
	gboolean recover, recov_folder;
	gint recon_delay;
};

#define TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_CAMEL_RECOVER_CONNECTION_STRATEGY, TnyCamelRecoverConnectionStrategyPriv))


static void
tny_camel_recover_connection_strategy_on_connect (TnyConnectionStrategy *self, TnyAccount *account)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (self);

	if (priv->folder && priv->recover && priv->recov_folder)
		tny_folder_refresh_async (priv->folder, NULL, NULL, NULL);

	priv->recover = FALSE;

	return;
}

static gboolean 
reconnect_it (gpointer user_data)
{
	TnyCamelAccount *account = (TnyCamelAccount *) user_data;
	tny_camel_account_set_online (account, TRUE, NULL, NULL);

	return FALSE;
}

static void 
reconnect_destroy (gpointer user_data)
{
	g_object_unref (user_data);
}

/**
 * tny_camel_recover_connection_strategy_set_recover_active_folder:
 * @self: a #TnyCamelRecoverConnectionStrategy instance
 * @setting: whether to recover the active folder
 * 
 * Sets whether to recover the active folder. Recovering the active folder means
 * that the folder will be refreshed once the account is back online, and if
 * the folder supports IDLE (in case of IMAP), its IDLE state is turned on.
 * Default value is TRUE.
 **/
void 
tny_camel_recover_connection_strategy_set_recover_active_folder (TnyCamelRecoverConnectionStrategy *self, gboolean setting)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (self);
	priv->recov_folder = setting;
}


/**
 * tny_camel_recover_connection_strategy_set_reconnect_delay:
 * @self: a #TnyCamelRecoverConnectionStrategy instance
 * @milliseconds: delay before a reconnect attempt happens, use -1 to disable
 * 
 * Sets the amount of milliseconds before a reconnect attempt takes place. Use
 * -1 to disable reconnecting. Default value is -1 or disabled.
 **/
void 
tny_camel_recover_connection_strategy_set_reconnect_delay (TnyCamelRecoverConnectionStrategy *self, gint milliseconds)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (self);
	priv->recon_delay = milliseconds;
}

static void
tny_camel_recover_connection_strategy_on_connection_broken (TnyConnectionStrategy *self, TnyAccount *account)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (self);

	priv->recover = TRUE;

	if (priv->recon_delay > 0) {
		g_timeout_add_full (G_PRIORITY_HIGH, priv->recon_delay, reconnect_it, 
			g_object_ref (account), reconnect_destroy);
	}

}

static void
tny_camel_recover_connection_strategy_on_disconnect (TnyConnectionStrategy *self, TnyAccount *account)
{
	return;
}

static void 
notify_folder_del (gpointer user_data, GObject *account)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (user_data);
	priv->folder = NULL;
}

static void
tny_camel_recover_connection_strategy_set_current (TnyConnectionStrategy *self, TnyAccount *account, TnyFolder *folder)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (self);
	if (priv->folder)
		g_object_weak_unref (G_OBJECT (priv->folder), notify_folder_del, self);
	priv->folder = folder;
	g_object_weak_ref (G_OBJECT (priv->folder), notify_folder_del, self);
}

static void
tny_camel_recover_connection_strategy_finalize (GObject *object)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (object);

	if (priv->folder)
		g_object_weak_unref (G_OBJECT (priv->folder), notify_folder_del, object);

	parent_class->finalize (object);
}

static void
tny_camel_recover_connection_strategy_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelRecoverConnectionStrategyPriv *priv = TNY_CAMEL_RECOVER_CONNECTION_STRATEGY_GET_PRIVATE (instance);
	priv->folder = NULL;
	priv->recover = FALSE;
	priv->recon_delay = -1;
	priv->recov_folder = TRUE;
}

static void
tny_connection_strategy_init (TnyConnectionStrategyIface *klass)
{
	klass->on_connect_func = tny_camel_recover_connection_strategy_on_connect;
	klass->on_connection_broken_func = tny_camel_recover_connection_strategy_on_connection_broken;
	klass->on_disconnect_func = tny_camel_recover_connection_strategy_on_disconnect;
	klass->set_current_func = tny_camel_recover_connection_strategy_set_current;
}

static void
tny_camel_recover_connection_strategy_class_init (TnyCamelRecoverConnectionStrategyClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_camel_recover_connection_strategy_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelRecoverConnectionStrategyPriv));

}

/**
 * tny_camel_recover_connection_strategy_new:
 * 
 * A connection strategy that tries to camel_recover the connection and the currently
 * selected folder.
 *
 * Return value: A new #TnyConnectionStrategy instance 
 **/
TnyConnectionStrategy*
tny_camel_recover_connection_strategy_new (void)
{
	return TNY_CONNECTION_STRATEGY (g_object_new (TNY_TYPE_CAMEL_RECOVER_CONNECTION_STRATEGY, NULL));
}


GType
tny_camel_recover_connection_strategy_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyCamelRecoverConnectionStrategyClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_camel_recover_connection_strategy_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyCamelRecoverConnectionStrategy),
			0,      /* n_preallocs */
			tny_camel_recover_connection_strategy_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_connection_strategy_info = 
		{
			(GInterfaceInitFunc) tny_connection_strategy_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelRecoverConnectionStrategy",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_CONNECTION_STRATEGY,
			&tny_connection_strategy_info);

	}
	return type;
}
