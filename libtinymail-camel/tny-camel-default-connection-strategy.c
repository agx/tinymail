/* Your copyright here */

#include <config.h>
#include <glib.h>
#include <glib/gi18n-lib.h>

#include <tny-camel-default-connection-strategy.h>

static GObjectClass *parent_class = NULL;

static void
tny_camel_default_connection_strategy_set_current (TnyConnectionStrategy *self, TnyAccount *account, TnyFolder *folder)
{
	return;
}

static void
tny_camel_default_connection_strategy_on_connect (TnyConnectionStrategy *self, TnyAccount *account)
{
	return;
}

static void
tny_camel_default_connection_strategy_on_connection_broken (TnyConnectionStrategy *self, TnyAccount *account)
{
	return;
}

static void
tny_camel_default_connection_strategy_on_disconnect (TnyConnectionStrategy *self, TnyAccount *account)
{
	return;
}

static void
tny_camel_default_connection_strategy_finalize (GObject *object)
{
	parent_class->finalize (object);
}

static void
tny_camel_default_connection_strategy_instance_init (GTypeInstance *instance, gpointer g_class)
{
}

static void
tny_connection_strategy_init (TnyConnectionStrategyIface *klass)
{
	klass->on_connect_func = tny_camel_default_connection_strategy_on_connect;
	klass->on_connection_broken_func = tny_camel_default_connection_strategy_on_connection_broken;
	klass->on_disconnect_func = tny_camel_default_connection_strategy_on_disconnect;
	klass->set_current_func = tny_camel_default_connection_strategy_set_current;
}

static void
tny_camel_default_connection_strategy_class_init (TnyCamelDefaultConnectionStrategyClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_camel_default_connection_strategy_finalize;
}



/**
 * tny_camel_default_connection_strategy_new:
 * 
 * A connection strategy that does nothing special
 *
 * Return value: A new #TnyConnectionStrategy instance 
 **/
TnyConnectionStrategy*
tny_camel_default_connection_strategy_new (void)
{
	return TNY_CONNECTION_STRATEGY (g_object_new (TNY_TYPE_CAMEL_DEFAULT_CONNECTION_STRATEGY, NULL));
}

GType
tny_camel_default_connection_strategy_get_type (void)
{
	static GType type = 0;
	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
			sizeof (TnyCamelDefaultConnectionStrategyClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_camel_default_connection_strategy_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyCamelDefaultConnectionStrategy),
			0,      /* n_preallocs */
			tny_camel_default_connection_strategy_instance_init,    /* instance_init */
			NULL
		};


		static const GInterfaceInfo tny_connection_strategy_info = 
		{
			(GInterfaceInitFunc) tny_connection_strategy_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelDefaultConnectionStrategy",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_CONNECTION_STRATEGY,
			&tny_connection_strategy_info);

	}
	return type;
}
