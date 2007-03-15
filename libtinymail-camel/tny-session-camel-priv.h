#ifndef TNY_SESSION_CAMEL_PRIV_H
#define TNY_SESSION_CAMEL_PRIV_H

struct _TnySessionCamelPriv
{
	gpointer device;
	gpointer account_store;
	gboolean interactive, prev_constat;
	guint connchanged_signal;
	GList *current_accounts;
	gchar *camel_dir;
	gboolean in_auth_function, is_connecting;
	gboolean async_connect;
	TnyLockable *ui_lock;
	GMutex *conlock;
	GThread *conthread;
	gboolean is_inuse;
};

void _tny_session_camel_add_account (TnySessionCamel *self, TnyCamelAccount *account);
void _tny_session_camel_forget_account (TnySessionCamel *self, TnyCamelAccount *account);

#endif
