#ifdef TNY_ENUMS_H

/* These are fakes for gtk-doc */

G_BEGIN_DECLS


typedef enum  
{
	TNY_FOLDER_STATUS_CODE_REFRESH = 1,
	TNY_FOLDER_STATUS_CODE_GET_MSG = 2,
	TNY_GET_MSG_QUEUE_STATUS_GET_MSG = 3
} TnyStatusCode;

typedef enum 
{
	TNY_FOLDER_STATUS = 1,
	TNY_GET_MSG_QUEUE_STATUS  = 2
} TnyStatusDomain;

typedef enum {
        TNY_FOLDER_CAPS_WRITABLE = 1<<0,
        TNY_FOLDER_CAPS_PUSHEMAIL = 1<<1
} TnyFolderCaps;

typedef enum {
	TNY_FOLDER_CHANGE_CHANGED_ALL_COUNT = 1<<1,
	TNY_FOLDER_CHANGE_CHANGED_UNREAD_COUNT = 1<<2,
	TNY_FOLDER_CHANGE_CHANGED_ADDED_HEADERS = 1<<3,
	TNY_FOLDER_CHANGE_CHANGED_REMOVED_HEADERS = 1<<4,
	TNY_FOLDER_CHANGE_CHANGED_FOLDER_RENAME = 1<<5
} TnyFolderChangeChanged;

typedef enum {
{
	TNY_FOLDER_STORE_CHANGE_CHANGED_CREATED_FOLDERS = 1<<1,
	TNY_FOLDER_STORE_CHANGE_CHANGED_REMOVED_FOLDERS = 1<<2
} TnyFolderStoreChangeChanged;

typedef enum {
	TNY_FOLDER_ERROR = 1,
	TNY_FOLDER_STORE_ERROR = 2,
	TNY_TRANSPORT_ACCOUNT_ERROR = 3,
	TNY_ACCOUNT_ERROR = 4
} TnyErrorDomain;

typedef enum {
	TNY_ERROR_UNSPEC = 1
	TNY_FOLDER_ERROR_SYNC = 2,
	TNY_FOLDER_ERROR_REMOVE_MSG = 3,
	TNY_FOLDER_ERROR_ADD_MSG = 4,
	TNY_FOLDER_ERROR_REFRESH = 5,
	TNY_FOLDER_ERROR_GET_MSG = 6,
	TNY_FOLDER_ERROR_TRANSFER_MSGS = 7,
	TNY_FOLDER_ERROR_SET_NAME = 8,
	TNY_FOLDER_ERROR_COPY = 9, 

	TNY_FOLDER_STORE_ERROR_REMOVE_FOLDER = 10,
	TNY_FOLDER_STORE_ERROR_GET_FOLDERS = 11,
	TNY_FOLDER_STORE_ERROR_CREATE_FOLDER = 12,

	TNY_TRANSPORT_ACCOUNT_ERROR_SEND = 13,

	TNY_ACCOUNT_ERROR_TRY_CONNECT = 14
} TnyError;

typedef enum  {
	TNY_ACCOUNT_TYPE_STORE,
	TNY_ACCOUNT_TYPE_TRANSPORT
} TnyAccountType;

typedef enum {
	TNY_ACCOUNT_STORE_ACCOUNT_CHANGED,
	TNY_ACCOUNT_STORE_ACCOUNT_INSERTED,
	TNY_ACCOUNT_STORE_ACCOUNT_REMOVED,
	TNY_ACCOUNT_STORE_ACCOUNTS_RELOADED,
	TNY_ACCOUNT_STORE_LAST_SIGNAL
} TnyAccountStoreSignal;

typedef enum  {
	TNY_ALERT_TYPE_INFO,
	TNY_ALERT_TYPE_WARNING,
	TNY_ALERT_TYPE_ERROR
} TnyAlertType;

typedef enum {
	TNY_ACCOUNT_STORE_TRANSPORT_ACCOUNTS,
	TNY_ACCOUNT_STORE_STORE_ACCOUNTS,
	TNY_ACCOUNT_STORE_BOTH
} TnyGetAccountsRequestType;

typedef enum {
	TNY_DEVICE_CONNECTION_CHANGED,
	TNY_DEVICE_LAST_SIGNAL
} TnyDeviceSignal;

typedef enum {
	TNY_FOLDER_FOLDER_INSERTED,
	TNY_FOLDER_FOLDERS_RELOADED,
	TNY_FOLDER_LAST_SIGNAL
} TnyFolderSignal;

typedef enum {
	TNY_FOLDER_STORE_QUERY_OPTION_SUBSCRIBED = 1<<1,
	TNY_FOLDER_STORE_QUERY_OPTION_UNSUBSCRIBED = 1<<2,
	TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_NAME = 1<<3,
	TNY_FOLDER_STORE_QUERY_OPTION_MATCH_ON_ID = 1<<4
} TnyFolderStoreQueryOption;

typedef enum {
	TNY_HEADER_FLAG_ANSWERED = 1<<0,
	TNY_HEADER_FLAG_DELETED = 1<<1,
	TNY_HEADER_FLAG_DRAFT = 1<<2,
	TNY_HEADER_FLAG_FLAGGED = 1<<3,
	TNY_HEADER_FLAG_SEEN = 1<<4,
	TNY_HEADER_FLAG_ATTACHMENTS = 1<<5,
	TNY_HEADER_FLAG_CACHED = 1<<6,
	TNY_HEADER_FLAG_PARTIAL = 1<<7,
	TNY_HEADER_FLAG_EXPUNGED = 1<<8,
	TNY_HEADER_FLAG_PRIORITY = 1<<9|1<<10
} TnyHeaderFlags;


typedef enum {
	TNY_HEADER_FLAG_HIGH_PRIORITY = 1<<9|1<<10,
	TNY_HEADER_FLAG_NORMAL_PRIORITY = 0<<9|0<<10,
	TNY_HEADER_FLAG_LOW_PRIORITY = 0<<9|1<<10
} TnyHeaderPriorityFlags;

typedef enum {
	TNY_FOLDER_TYPE_UNKNOWN,
	TNY_FOLDER_TYPE_NORMAL,
	TNY_FOLDER_TYPE_INBOX,
	TNY_FOLDER_TYPE_OUTBOX,
	TNY_FOLDER_TYPE_TRASH,
	TNY_FOLDER_TYPE_JUNK,
	TNY_FOLDER_TYPE_SENT,
	TNY_FOLDER_TYPE_ROOT
} TnyFolderType;

G_END_DECLS

#endif
