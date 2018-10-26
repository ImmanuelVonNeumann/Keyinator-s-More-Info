/*
 * TeamSpeak 3 demo plugin
 *
 * Copyright (c) 2008-2017 TeamSpeak Systems GmbH
 */

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"
#include <string>

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if(!result) {
		const wchar_t* name = L"Keyinator's More Info";
		if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = "Keyinator's More Info";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Keyinator's More Info";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
    return "1.1";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "Keyinator";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "A plugin to give more infos to users";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {
    char appPath[PATH_BUFSIZE];
    char resourcesPath[PATH_BUFSIZE];
    char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

    /* Your plugin init code here */
    printf("PLUGIN: init\n");

    /* Example on how to query application, resources and configuration paths from client */
    /* Note: Console client returns empty string for app and resources path */
    ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
    ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
    ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

	printf("PLUGIN: App path: %s\nResources path: %s\nConfig path: %s\nPlugin path: %s\n", appPath, resourcesPath, configPath, pluginPath);

    return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
    /* Your plugin cleanup code here */
    printf("PLUGIN: shutdown\n");

	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	printf("PLUGIN: offersConfigure\n");
	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
	return PLUGIN_OFFERS_NO_CONFIGURE;  /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
    printf("PLUGIN: configure\n");
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "Extend info";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {

	/* For demonstration purpose, display the name of the currently selected server, channel or client. */

#pragma warning( push )
#pragma warning( disable : 4129)

	std::string infodata;
	bool fail = false;
	switch(type) {
		case PLUGIN_SERVER:

			ts3Functions.requestServerVariables(serverConnectionHandlerID);

			char* server_uid;
			char* server_name;
			char* server_wm;
			char* server_platform;
			char* server_version;
			char* server_mc;
			char* server_co;
			char* server_created;
			char* server_uptime;
			char* server_cem;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_UNIQUE_IDENTIFIER, &server_uid);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &server_name);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_PLATFORM, &server_platform);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_VERSION, &server_version);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MAXCLIENTS, &server_mc);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_CLIENTS_ONLINE, &server_co);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_CREATED, &server_created);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_UPTIME, &server_uptime);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_CODEC_ENCRYPTION_MODE, &server_cem);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);

			infodata += "Server-UID: [B]";
			infodata += server_uid;
			infodata += "[/B]\n";
			infodata += "Server-NAME: [B]";
			infodata += server_name;
			infodata += "[/B]\n";
			infodata += "Server-PLATFORM: [B]";
			infodata += server_platform;
			infodata += "[/B]\n";
			infodata += "Server-VERSION: [B]";
			infodata += server_version;
			infodata += "[/B]\n";
			infodata += "Server-CLIENTS: [B]";
			infodata += server_co;
			infodata += " / ";
			infodata += server_mc;
			infodata += "[/B]\n";
			infodata += "Server-CREATED: [B]";
			infodata += server_created;
			infodata += "[/B]\n";
			infodata += "Server-CODEC_ENCRYPTION_MODE: [B]";
			infodata += server_cem;
			infodata += "[/B]\n";
			infodata += "Server-WELCOME MESSAGE: UNDERNEATH\n";
			infodata += "\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/[B]\n";
			infodata += server_wm;
			infodata += "\n[/B]/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\";
			infodata += "\n\n[B][U]EXTENDED[/U][/B]\n\n";

			//---------------------------------------------------------------------------

			char* server_d_sg;
			char* server_d_cg;
			char* server_d_cag;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DEFAULT_SERVER_GROUP, &server_d_sg);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DEFAULT_CHANNEL_GROUP, &server_d_cg);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DEFAULT_CHANNEL_ADMIN_GROUP, &server_d_cag);
			
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &server_wm);

			infodata += "DEFAULT-GROUPS: [B]";
			infodata += "DEFAULT_SERVER_GROUP: [B]";
			infodata += server_d_sg;
			infodata += "[/B]\n";
			infodata += "DEFAULT_CHANNEL_GROUP: [B]";
			infodata += server_d_cg;
			infodata += "[/B]\n";
			infodata += "DEFAULT_CHANNEL_ADMIN_GROUP: [B]";
			infodata += server_d_cag;
			infodata += "[/B]\n";
			/*
			infodata += "DEFAULT_CHANNEL_GROUP: [B]";
			infodata += server_uid;
			infodata += "[/B]\n";
			infodata += "DEFAULT_CHANNEL_GROUP: [B]";
			infodata += server_uid;
			infodata += "[/B]\n";
			*/



			/*
			VIRTUALSERVER_HOSTMESSAGE_MODE
				VIRTUALSERVER_FLAG_PASSWORD
				VIRTUALSERVER_MAX_DOWNLOAD_TOTAL_BANDWIDTH
				VIRTUALSERVER_MAX_UPLOAD_TOTAL_BANDWIDTH
				VIRTUALSERVER_HOSTBANNER_URL
				VIRTUALSERVER_HOSTBANNER_GFX_URL
				VIRTUALSERVER_HOSTBANNER_GFX_INTERVAL
				VIRTUALSERVER_COMPLAIN_AUTOBAN_COUNT
				VIRTUALSERVER_COMPLAIN_AUTOBAN_TIME
				VIRTUALSERVER_COMPLAIN_REMOVE_TIME
				VIRTUALSERVER_MIN_CLIENTS_IN_CHANNEL_BEFORE_FORCED_SILENCE
				VIRTUALSERVER_PRIORITY_SPEAKER_DIMM_MODIFICATOR
				VIRTUALSERVER_ID
				VIRTUALSERVER_ANTIFLOOD_POINTS_TICK_REDUCE
				VIRTUALSERVER_ANTIFLOOD_POINTS_NEEDED_COMMAND_BLOCK
				VIRTUALSERVER_ANTIFLOOD_POINTS_NEEDED_IP_BLOCK
				VIRTUALSERVER_CLIENT_CONNECTIONS
				VIRTUALSERVER_QUERY_CLIENT_CONNECTIONS
				VIRTUALSERVER_HOSTBUTTON_TOOLTIP
				VIRTUALSERVER_HOSTBUTTON_URL
				VIRTUALSERVER_HOSTBUTTON_GFX_URL,                          //available when connected, always up-to-date 

				VIRTUALSERVER_QUERYCLIENTS_ONLINE
				VIRTUALSERVER_DOWNLOAD_QUOTA
				VIRTUALSERVER_UPLOAD_QUOTA
				VIRTUALSERVER_MONTH_BYTES_DOWNLOADED
				VIRTUALSERVER_MONTH_BYTES_UPLOADED,
				VIRTUALSERVER_TOTAL_BYTES_DOWNLOADED
				VIRTUALSERVER_TOTAL_BYTES_UPLOADED
				VIRTUALSERVER_PORT
				VIRTUALSERVER_AUTOSTART
				VIRTUALSERVER_MACHINE_ID
				VIRTUALSERVER_NEEDED_IDENTITY_SECURITY_LEVEL
				VIRTUALSERVER_MIN_CLIENT_VERSION
				VIRTUALSERVER_NAME_PHONETIC
				VIRTUALSERVER_ICON_ID
				VIRTUALSERVER_RESERVED_SLOTS
				VIRTUALSERVER_IP
				VIRTUALSERVER_TOTAL_PING
				VIRTUALSERVER_WEBLIST_ENABLED
				VIRTUALSERVER_ASK_FOR_PRIVILEGEKEY
				VIRTUALSERVER_HOSTBANNER_MODE
				VIRTUALSERVER_CHANNEL_TEMP_DELETE_DELAY_DEFAULT
				VIRTUALSERVER_MIN_ANDROID_VERSION
				VIRTUALSERVER_MIN_IOS_VERSION
				VIRTUALSERVER_MIN_WINPHONE_VERSION
				*/

			//snprintf(*data, INFODATA_BUFSIZE, "Server-UID: [I]\%s\[/I]", server_uid);  /* bbCode is supported. HTML is not supported */

			break;

		case PLUGIN_CHANNEL:

			char* channel_name;
			char* channel_order;
			char* channel_delete_delay;

			ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_NAME, &channel_name);
			ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_ORDER, &channel_order);
			ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_DELETE_DELAY, &channel_delete_delay);

			infodata += "Channel-NAME: [I]";
			infodata += channel_name;
			infodata += "[/I]\n";
			infodata += "Channel-ORDER: [I]";
			infodata += channel_order;
			infodata += "[/I]\n";
			infodata += "Channel-DELETE-DELAY [I]";
			infodata += channel_delete_delay;
			infodata += "[/I]\n";
			
			//snprintf(*data, INFODATA_BUFSIZE, "Channel-NAME: [I]\%s\[/I]\n\Channel-ORDER: [I]\%s\[/I]\n\Channel-DELETE-DELAY: [I]\%s\[/I]", channel_name, channel_order, channel_delete_delay);  /* bbCode is supported. HTML is not supported */

			break;

		case PLUGIN_CLIENT:

			char* client_name;
			char* client_uid;
			char* client_version;

			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &client_name);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNIQUE_IDENTIFIER, &client_uid);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_VERSION, &client_version);

			infodata += "Client - NAME: [I]";
			infodata += client_name;
			infodata += "[/I]\n";
			infodata += "Client-UID: [I]";
			infodata += client_uid;
			infodata += "[/I]\n";
			infodata += "Client-VERSION: [I]";
			infodata += client_version;
			infodata += "[/I]\n";

			//snprintf(*data, INFODATA_BUFSIZE, "Client-NAME: [I]\%s\[/I]\n\Client-UID: [I]\%s\[/I]\n\Client-VERSION: [I]\%s\[/I]", client_name, client_uid, client_version);  /* bbCode is supported. HTML is not supported */
			
			break;

		default:
			printf("Invalid item type: %d\n", type);
			//data = NULL;  /* Ignore */
			fail = true;
			return;
	}
	if (!fail) {
		*data = (char*)malloc((infodata.length() + 1) * sizeof(char));
		snprintf(*data, (infodata.length() + 1), infodata.c_str());
	}
#pragma warning( pop )
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
	return 0;  /* 1 = request autoloaded, 0 = do not request autoload */
}