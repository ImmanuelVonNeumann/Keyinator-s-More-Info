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
#include "functions.h"
#include "badge_ids.h"

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
		const wchar_t* name = L"!Keyinator's More Info";
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
    return "1.3";
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
	init_guids();

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
	return "Keyinator's More Info";
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

			char* server_id;
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

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_ID, &server_id);
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

			infodata += "Server-NAME: [B]";
			infodata += server_name;
			infodata += "[/B]\n";
			infodata += "Server-ID: [B]";
			infodata += server_id;
			infodata += "[/B]\n";
			infodata += "Server-UID: [B]";
			infodata += server_uid;
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
			infodata += get_time_string(atoi(server_created));
			infodata += "[/B]\n";
			infodata += "Server-CODEC_ENCRYPTION_MODE: [B]";
			infodata += server_cem;
			infodata += "[/B]\n";
			infodata += "Server-WELCOME MESSAGE: [B]UNDERNEATH[/B]\n";
			infodata += "\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/[B]\n";
			infodata += server_wm;
			infodata += "\n[/B]/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\";
			infodata += "\n\n\n[B][U]EXTENDED[/U][/B]\n\n";

			//---------------------------------------------------------------------------

			char* server_d_sg;
			char* server_d_cg;
			char* server_d_cag;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DEFAULT_SERVER_GROUP, &server_d_sg);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DEFAULT_CHANNEL_GROUP, &server_d_cg);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DEFAULT_CHANNEL_ADMIN_GROUP, &server_d_cag);

			infodata += "[B]DEFAULT-GROUPS:[/B]\n";
			infodata += "DEFAULT_SERVER_GROUP: [B]";
			infodata += server_d_sg;
			infodata += "[/B]\n";
			infodata += "DEFAULT_CHANNEL_GROUP: [B]";
			infodata += server_d_cg;
			infodata += "[/B]\n";
			infodata += "DEFAULT_CHANNEL_ADMIN_GROUP: [B]";
			infodata += server_d_cag;
			infodata += "[/B]\n\n";

			char* server_bw_up;
			char* server_bw_down;
			
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MAX_UPLOAD_TOTAL_BANDWIDTH, &server_bw_up);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MAX_DOWNLOAD_TOTAL_BANDWIDTH, &server_bw_down);

			infodata += "[B]TOTAL BANDWIDTH:[/B]\n";
			infodata += "UP: [B]";
			infodata += std::to_string(atoi(server_bw_up) / 1000 / 1000 / 1000);
			infodata += " GBYTE | ";
			infodata += std::to_string(atoi(server_bw_up) / 1000 / 1000);
			infodata += " MBYTE | ";
			infodata += std::to_string(atoi(server_bw_up) / 1000);
			infodata += " KBYTE | ";
			infodata += std::to_string(atoi(server_bw_up));
			infodata += " BYTE[/B]\n";
			infodata += "DOWN: [B]";
			infodata += std::to_string(atoi(server_bw_down) / 1000 / 1000 / 1000);
			infodata += " GBYTE | ";
			infodata += std::to_string(atoi(server_bw_down) / 1000 / 1000);
			infodata += " MBYTE | ";
			infodata += std::to_string(atoi(server_bw_down) / 1000);
			infodata += " KBYTE | ";
			infodata += std::to_string(atoi(server_bw_down));
			infodata += " BYTE[/B]\n\n";


		/* CRASHES CLIENT
			char* hostbanner_url;
			char* hostbanner_gfx_url;
			char* hostbanner_gfx_interval;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_HOSTBANNER_URL, &hostbanner_url);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_HOSTBANNER_GFX_URL, &hostbanner_gfx_url);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_HOSTBANNER_GFX_INTERVAL, &hostbanner_gfx_interval);

			infodata += "[B]HOSTBANNER:[/B]\n";
			infodata += "HOSTBANNER-LINK: [B]";
			infodata += hostbanner_url;
			infodata += "[/B]\n";
			infodata += "HOSTBANNER-PICTURE: [B]";
			infodata += hostbanner_gfx_url;
			infodata += "[/B]\n";
			infodata += "HOSTBANNER-UPDATE-INTERVAL: [B]";
			infodata += hostbanner_gfx_interval;
			infodata += "[/B]\n\n";

		*/

			char* hostbutton_url;
			char* hostbutton_gfx_url;
			char* hostbutton_gfx_interval;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_HOSTBUTTON_TOOLTIP, &hostbutton_url);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_HOSTBUTTON_URL, &hostbutton_gfx_url);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_HOSTBUTTON_GFX_URL, &hostbutton_gfx_interval);

			infodata += "[B]HOSTBUTTON:[/B]\n";
			infodata += "HOSTBUTTON-TOOLTIP: [B]";
			infodata += hostbutton_url;
			infodata += "[/B]\n";
			infodata += "HOSTBUTTON-LINK: [B]";
			infodata += hostbutton_gfx_url;
			infodata += "[/B]\n";
			infodata += "HOSTBUTTON-IMAGE: [B]";
			infodata += hostbutton_gfx_interval;
			infodata += "[/B]\n\n";

			

			char* vs_min_client;
			char* vs_min_android;
			char* vs_min_ios;
			char* vs_min_winphone;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MIN_CLIENT_VERSION, &vs_min_client);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MIN_ANDROID_VERSION, &vs_min_android);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MIN_IOS_VERSION, &vs_min_ios);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_MIN_WINPHONE_VERSION, &vs_min_winphone);

			infodata += "[B]MINIMUM REQUIREMENTS:[/B]\n";
			infodata += "CLIENT: [B]";
			infodata += vs_min_client;
			infodata += "[/B]\n";
			infodata += "ANDROID: [B]";
			infodata += vs_min_android;
			infodata += "[/B]\n";
			infodata += "IOS: [B]";
			infodata += vs_min_ios;
			infodata += "[/B]\n";
			infodata += "WINPHONE: [B]";
			infodata += vs_min_winphone;
			infodata += "[/B]\n\n";
		
			char* ip;
			char* ip_port;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_IP, &ip);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_PORT, &ip_port);

			infodata += "[B]SERVER-IP:[/B]\n[B]";
			infodata += ip;
			infodata += ":";
			infodata += ip_port;
			infodata += "[/B]\n\n";

			char* complain_ab_count;
			char* complain_ab_time;
			char* complain_ab_rm_time;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_COMPLAIN_AUTOBAN_COUNT, &complain_ab_count);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_COMPLAIN_AUTOBAN_TIME, &complain_ab_time);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_COMPLAIN_REMOVE_TIME, &complain_ab_rm_time);

			infodata += "[B]COMPLAINS:[/B]\n";
			infodata += "COUNT TO BAN: [B]";
			infodata += complain_ab_count;
			infodata += "[/B]\n";
			infodata += "BAN TIME: [B]";
			infodata += complain_ab_time;
			infodata += " sec[/B]\n";
			infodata += "REMOVE COMPLAINS AFTER: [B]";
			infodata += complain_ab_rm_time;
			infodata += " sec[/B]\n\n";

			char* server_quota_up;
			char* server_quota_down;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_UPLOAD_QUOTA, &server_quota_up);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_DOWNLOAD_QUOTA, &server_quota_down);

			infodata += "[B]TOTAL BANDWIDTH:[/B]\n";
			infodata += "UP: [B]";
			infodata += std::to_string(atoi(server_quota_up) / 1000 / 1000 / 1000);
			infodata += " GBYTE | ";
			infodata += std::to_string(atoi(server_quota_up) / 1000 / 1000);
			infodata += " MBYTE | ";
			infodata += std::to_string(atoi(server_quota_up) / 1000);
			infodata += " KBYTE | ";
			infodata += std::to_string(atoi(server_quota_up));
			infodata += " BYTE[/B]\n";
			infodata += "DOWN: [B]";
			infodata += std::to_string(atoi(server_quota_down) / 1000 / 1000 / 1000);
			infodata += " GBYTE | ";
			infodata += std::to_string(atoi(server_quota_down) / 1000 / 1000);
			infodata += " MBYTE | ";
			infodata += std::to_string(atoi(server_quota_down) / 1000);
			infodata += " KBYTE | ";
			infodata += std::to_string(atoi(server_quota_down));
			infodata += " BYTE[/B]\n\n";


			char* complain_af_reduce;
			char* complain_af_cmd_block;
			char* complain_af_ip_block;

			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_ANTIFLOOD_POINTS_TICK_REDUCE, &complain_af_reduce);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_ANTIFLOOD_POINTS_NEEDED_COMMAND_BLOCK, &complain_af_cmd_block);
			ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_ANTIFLOOD_POINTS_NEEDED_IP_BLOCK, &complain_af_ip_block);

			infodata += "[B]ANITFLOOD:[/B]\n";
			infodata += "POINTS REDUCED PER TICK: [B]";
			infodata += complain_af_reduce;
			infodata += "[/B]\n";
			infodata += "TICKS UNTIL COMMAND BLOCK: [B]";
			infodata += complain_af_cmd_block;
			infodata += " sec[/B]\n";
			infodata += "TICKS UNTIL IP BLOCK: [B]";
			infodata += complain_af_ip_block;
			infodata += " sec[/B]\n\n";
			
			
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
				VIRTUALSERVER_MIN_CLIENTS_IN_CHANNEL_BEFORE_FORCED_SILENCE
				VIRTUALSERVER_PRIORITY_SPEAKER_DIMM_MODIFICATOR
				VIRTUALSERVER_IDK
				VIRTUALSERVER_CLIENT_CONNECTIONS
				VIRTUALSERVER_QUERY_CLIENT_CONNECTIONS

				VIRTUALSERVER_QUERYCLIENTS_ONLINE
				VIRTUALSERVER_DOWNLOAD_QUOTA
				VIRTUALSERVER_UPLOAD_QUOTA
				VIRTUALSERVER_MONTH_BYTES_DOWNLOADED
				VIRTUALSERVER_MONTH_BYTES_UPLOADED,
				VIRTUALSERVER_TOTAL_BYTES_DOWNLOADED
				VIRTUALSERVER_TOTAL_BYTES_UPLOADED
				VIRTUALSERVER_AUTOSTART
				VIRTUALSERVER_MACHINE_ID
				VIRTUALSERVER_NEEDED_IDENTITY_SECURITY_LEVEL
				VIRTUALSERVER_NAME_PHONETIC
				VIRTUALSERVER_ICON_ID
				VIRTUALSERVER_RESERVED_SLOTS
				VIRTUALSERVER_TOTAL_PING
				VIRTUALSERVER_WEBLIST_ENABLED
				VIRTUALSERVER_ASK_FOR_PRIVILEGEKEY
				VIRTUALSERVER_HOSTBANNER_MODE
				VIRTUALSERVER_CHANNEL_TEMP_DELETE_DELAY_DEFAULT
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

			ts3Functions.requestServerVariables(serverConnectionHandlerID);
			ts3Functions.requestClientVariables(serverConnectionHandlerID, (anyID)id, NULL);

			char* client_name;
			char* client_uid;
			char* client_version;

			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &client_name);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNIQUE_IDENTIFIER, &client_uid);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_VERSION, &client_version);

			

		//v1.3 CHANGES

			char* client_platform;
			// Not working char* client_talking;
			char* client_input_muted;
			char* client_output_muted;
			// Not working char* client_outputonly_muted;
			char* client_input_hw;
			char* client_output_hw;

			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_PLATFORM, &client_platform);
			//ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_FLAG_TALKING, &client_talking);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_INPUT_MUTED, &client_input_muted);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_OUTPUT_MUTED, &client_output_muted);
			//ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_OUTPUTONLY_MUTED, &client_outputonly_muted);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_INPUT_HARDWARE, &client_input_hw);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_OUTPUT_HARDWARE, &client_output_hw);

			;

			char* client_idle;
			char* client_muted;
			char* client_recording;

			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_IDLE_TIME, &client_idle);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_IS_MUTED, &client_muted);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_IS_RECORDING, &client_recording);

			

			char* client_dbid;
			char* client_channel_group_id;
			char* client_server_groups;
			char* client_created;

			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_DATABASE_ID, &client_dbid);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_CHANNEL_GROUP_ID, &client_channel_group_id);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_SERVERGROUPS, &client_server_groups);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_CREATED, &client_created);

			

			char* client_connections;
			char* client_away;
			char* client_away_message;
			char* client_avatar;
			char* client_tp;
			char* client_talk_request;
			char* client_talk_request_msg;
			char* client_description;
			char* client_talker;
			char* client_priority_speaker;
			char* client_unread_messages;
			char* client_phonetic_nickname;
			char* client_icon_id;
			char* client_channel_commander;
			char* client_country;
			char* client_badges;

			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_TOTALCONNECTIONS, &client_connections);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_AWAY, &client_away);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_AWAY_MESSAGE, &client_away_message);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_FLAG_AVATAR, &client_avatar);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_TALK_POWER, &client_tp);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_TALK_REQUEST, &client_talk_request);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_TALK_REQUEST_MSG, &client_talk_request_msg);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_DESCRIPTION, &client_description);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_IS_TALKER, &client_talker);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_IS_PRIORITY_SPEAKER, &client_priority_speaker);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_UNREAD_MESSAGES, &client_unread_messages);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME_PHONETIC, &client_phonetic_nickname);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_ICON_ID, &client_icon_id);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_IS_CHANNEL_COMMANDER, &client_channel_commander);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_COUNTRY, &client_country);
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_BADGES, &client_badges);

			


			//---------------------------------------------------------------------------

			infodata += "CLIENT-RELATED:";
			infodata += "\n";
			infodata += "------------------------";
			infodata += "\n";
			
			infodata += "name: [I]";
			infodata += client_name;
			infodata += "[/I]\n";
			infodata += "uuid: [I]";
			infodata += client_uid;
			infodata += "[/I]\n";
			infodata += "build: [I]";
			infodata += client_version;
			infodata += " on ";
			infodata += client_platform;
			infodata += "[/I]\n";
			infodata += "client phonetic name: [I]";
			infodata += client_phonetic_nickname;
			infodata += "[/I]\n";
			infodata += "country of client: [I]";
			infodata += client_country;
			infodata += "[/I]\n";
			infodata += "badges of client:";
			{
				std::vector<std::string> arr = split(client_badges, ':');
				if (!arr.empty()) {
					if (arr[0] == "overwolf=0") {
						infodata += "";
					}
					else {
						infodata += " [I]Overwolf[/I]";
					}

					if (arr.size() > 1) {
						std::vector<std::string> arr2 = split(arr[1], ',');
						//erase(arr2, 0, 0, 6);
						arr2[0] = arr2[0].erase(0, 7);
						//printf(arr[1].c_str());
						for (std::vector<std::string>::iterator it = arr2.begin(); it != arr2.end(); it++) {
							//printf(it->c_str());
							infodata += " | [I]";
							infodata += guid_name(*it);
							infodata += "[/I]";
						}
						infodata += "\n";
					}
				}

			}


			infodata += "\n";
			infodata += "STATUS:";
			infodata += "\n";
			//infodata += "Is client talking: [I]";
			//infodata += client_talking;
			//infodata += "[/I]\n";
			//infodata += "Is client's microphone muted: [I]";
			//infodata += client_input_muted;
			//infodata += "[/I]\n";
			//infodata += "Is client's headset muted: [I]";
			//infodata += client_output_muted;
			//infodata += "[/I]\n";
			////infodata += "Is client's outputonly muted: [I]";
			////infodata += client_outputonly_muted;
			////infodata += "[/I]\n";
			//infodata += "Is client's microphone activated: [I]";
			//infodata += client_input_hw;
			//infodata += "[/I]\n";
			//infodata += "Is client's headset activated: [I]";
			//infodata += client_output_hw;
			//infodata += "[/I]\n";
			//infodata += "Is client away: [I]";
			//infodata += client_away;
			//infodata += "[/I]\n";
			/*infodata += "Away-Message: [I]";
			infodata += client_away_message;
			infodata += "[/I]\n";*/
			infodata += "Has client requested tp: [I]";
			infodata += client_talk_request;
			infodata += "[/I]\n";
			/*infodata += "talkpower request message: [I]";
			infodata += client_talk_request_msg;
			infodata += "[/I]\n";*/
			infodata += "Client-Idle-Time: [I]";
			infodata += client_idle;
			infodata += "[/I]\n";
			infodata += "Client-Muted (by you): [I]";
			infodata += client_muted;
			infodata += "[/I]\n";
			infodata += "Is client recording: [I]";
			infodata += client_recording;
			infodata += "[/I]\n";
			infodata += "\n";

			//---------------

			infodata += "SERVER-RELATED:";
			infodata += "\n";
			infodata += "------------------------";
			infodata += "\n";

			infodata += "databaseID: [I]";
			infodata += client_dbid;
			infodata += "[/I]\n";
			infodata += "connections to server: [I]";
			infodata += client_connections;
			infodata += "[/I]\n";
			infodata += "First connection of client: [I]";
			infodata += get_time_string(atoi(client_created));;
			infodata += "[/I]\n";

			infodata += "\n";
			infodata += "GROUPS: [I]";
			infodata += "[/I]\n";
			infodata += "\n";
			infodata += "servergroupid(s): [I]";
			infodata += client_server_groups;
			infodata += "[/I]\n";
			infodata += "channelgroupid: [I]";
			infodata += client_channel_group_id;
			infodata += "[/I]\n";

			//---------------

			infodata += "\n";
			infodata += "PERMS:";
			infodata += "\n";
			infodata += "------------------------";
			infodata += "\n";

			infodata += "client talkpower: [I]";
			infodata += client_tp;
			infodata += "[/I]\n";
			infodata += "client avatar id: [I]";
			infodata += client_avatar;
			infodata += "[/I]\n";
			infodata += "client icon id: [I]";
			infodata += client_icon_id;
			infodata += "[/I]\n";
			infodata += "client is talker: [I]";
			infodata += client_talker;
			infodata += "[/I]\n";
			infodata += "client is priority speaker: [I]";
			infodata += client_priority_speaker;
			infodata += "[/I]\n";
			infodata += "unread messages clientside: [I]";
			infodata += client_unread_messages;
			infodata += "[/I]\n";
			infodata += "client channel commander: [I]";
			infodata += client_channel_commander;
			infodata += "[/I]\n";

			//---------------

			/*infodata += "Client description: [I]";
			infodata += client_description;
			infodata += "[/I]\n";*/
			
			


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