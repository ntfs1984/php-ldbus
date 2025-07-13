#include <phpcpp.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <map>
#include <string>
#include <bits/stdc++.h>
#include <dbus/dbus.h>

using namespace std;
class Ldbus : public Php::Base {
private:
DBusConnection* connection;
DBusMessage* message_dbus;
DBusMessage *reply_dbus;
DBusMessageIter iter_dbus;
DBusMessageIter dict_iter_dbus;
DBusMessageIter entry_iter_dbus;
  
bool verbose = false;
int timeout = 250;
public:
    Php::Value __construct(Php::Parameters &params);
    Php::Value bus_request_name(Php::Parameters &params);
    Php::Value connection_pop_message(Php::Parameters &params);
    Php::Value connection_send(Php::Parameters &params);
    Php::Value connection_send_with_reply_and_block(Php::Parameters &params);
    Php::Value connection_flush(Php::Parameters &params);
    Php::Value message_get_type(Php::Parameters &params);
    Php::Value message_get_destination(Php::Parameters &params);
    Php::Value message_get_path(Php::Parameters &params);
    Php::Value message_new_method_return(Php::Parameters &params);
    Php::Value message_get_interface(Php::Parameters &params);
    Php::Value message_get_member(Php::Parameters &params);
    Php::Value message_get_parameters(Php::Parameters &params);
    Php::Value message_is_method_call(Php::Parameters &params);
    Php::Value message_iter_init_append(Php::Parameters &params);
    Php::Value message_iter_append_basic(Php::Parameters &params);
    Php::Value message_new_method_call(Php::Parameters &params);
    Php::Value test_function(Php::Parameters &params);
};

Php::Value Ldbus::__construct(Php::Parameters &params)    {
	//std::string type = params[0];
	int bus = (int)params[0];
	if (bus==DBUS_BUS_SESSION) {connection = dbus_bus_get(DBUS_BUS_SESSION, NULL);return connection;}
	if (bus==DBUS_BUS_SYSTEM) {connection = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);return connection;}
	printf("Usage: $dbus = new Dbus(const bus); // Bus could be const G_BUS_TYPE_SESSION or G_BUS_TYPE_SYSTEM \n");	
	return false;
} 


Php::Value Ldbus::bus_request_name(Php::Parameters &params) {
	const char *bus = params[0];
	DBusError err;
	dbus_error_init(&err);
	return (guint *)dbus_bus_request_name(connection, bus, DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	
}

Php::Value Ldbus::connection_pop_message(Php::Parameters &params) {
	dbus_connection_read_write(connection, 0);
	message_dbus = dbus_connection_pop_message(connection);
	if (message_dbus == NULL) {return false;} else {return true;}
}

Php::Value Ldbus::connection_send(Php::Parameters &params) {
	if (reply_dbus==NULL) {
			return false;
	}
	dbus_connection_send(connection, reply_dbus, NULL);
	return true;
}


Php::Value Ldbus::connection_send_with_reply_and_block(Php::Parameters &params) {
	if (reply_dbus==NULL) {
			return false;
	}
	message_dbus = dbus_connection_send_with_reply_and_block(connection, reply_dbus, -1, NULL);
	return true;
}

Php::Value Ldbus::connection_flush(Php::Parameters &params) {
	dbus_connection_flush(connection);
	if (reply_dbus!=NULL) {dbus_message_unref(reply_dbus); return true;}
	return false;
}

Php::Value Ldbus::message_get_path(Php::Parameters &params) {
	const char *path = dbus_message_get_path(message_dbus);
	return path;
}

Php::Value Ldbus::message_get_destination(Php::Parameters &params) {
	const char *dest = dbus_message_get_destination(message_dbus);
	return dest;
}

Php::Value Ldbus::message_get_type(Php::Parameters &params) {
	int type = dbus_message_get_type(message_dbus);
	return type;
}

Php::Value Ldbus::message_get_interface(Php::Parameters &params) {
	const char *interf = dbus_message_get_interface(message_dbus);
	return interf;
}

Php::Value Ldbus::message_get_member(Php::Parameters &params) {
	const char *member = dbus_message_get_member(message_dbus);
	return member;
}

Php::Value Ldbus::message_is_method_call(Php::Parameters &params) {
	const char *interface = params[0];
	const char *method = params[1];
	if (dbus_message_is_method_call(message_dbus, interface, method)) {return true;} else {return false;}
}

Php::Value Ldbus::message_iter_append_basic(Php::Parameters &params) {
	int message_type = params[0];
		switch (message_type) {
            case DBUS_TYPE_STRING: {
				const char *string = params[1];
				dbus_message_iter_append_basic(&iter_dbus, DBUS_TYPE_STRING, &string);
                break;
            }
            case DBUS_TYPE_UINT32: {
				int num = (int)params[1];
				dbus_message_iter_append_basic(&iter_dbus, DBUS_TYPE_UINT32, &num); // DBUS_TYPE_UINT32
                break;
            }
            case DBUS_TYPE_DOUBLE: {
                break;
            }
            case DBUS_TYPE_ARRAY: {
				//Php::Value string_array = params[1];	
				const char **string_array;
				int array_length = (int)params[2];
				DBusMessageIter array_iter;
				DBusError error;
				dbus_error_init(&error);
				if (!dbus_message_iter_open_container(&iter_dbus, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array_iter)) {
					fprintf(stderr, "php-ldbus: Error opening array\n");
					return false;
			    }
				for (int i = 0; i < array_length; ++i) {
					if (!dbus_message_iter_append_basic(&array_iter, DBUS_TYPE_STRING, &string_array[i])) {
						fprintf(stderr, "php-ldbus: Error adding string to array\n");
						return false;
					}
				}
				if (!dbus_message_iter_close_container(&iter_dbus, &array_iter)) {
					fprintf(stderr, "php-ldbus: Unable to close array container\n");
					return false;
				}
                break;
            }
            default:
		       printf("php-ldbus: Invalid argument of type: %d \n", message_type);
                break;
        }
	return true;
}


Php::Value Ldbus::message_get_parameters(Php::Parameters &params) {
	DBusMessageIter iter;
	Php::Value array;
	int parm = 0;
	int message_type = dbus_message_get_type(message_dbus);
	dbus_message_iter_init(message_dbus, &iter);
	while ((message_type = dbus_message_iter_get_arg_type(&iter)) != DBUS_TYPE_INVALID) {
		switch (message_type) {
            case DBUS_TYPE_STRING: {
                char *arg_string;
                dbus_message_iter_get_basic(&iter, &arg_string);
                parm++;
                array[parm] = arg_string;
                break;
            }
            case DBUS_TYPE_INT32: {
                int arg_int;
                dbus_message_iter_get_basic(&iter, &arg_int);
                parm++;
                array[parm] = arg_int;
                break;
            }
            case DBUS_TYPE_DOUBLE: {
                double arg_double;
                dbus_message_iter_get_basic(&iter, &arg_double);
                parm++;
				array[parm] = arg_double;
                break;
            }
            default:
				array[parm] = false;
                printf("php-ldbus: Invalid argument of type: %d \n", message_type);
                break;
        }
		dbus_message_iter_next(&iter);
	}
	return array;
}


Php::Value Ldbus::message_new_method_return(Php::Parameters &params) {
	if (message_dbus==NULL) {printf("php-ldbus: message_dbus is null, seems you should use connection_pop_message() before \n");return false;}
	reply_dbus = dbus_message_new_method_return(message_dbus);
	return true;
}

Php::Value Ldbus::message_iter_init_append(Php::Parameters &params) {
	if (reply_dbus==NULL) {printf("php-ldbus: message_iter_init_append error, reply_dbus is null \n");return false;}
	dbus_message_iter_init_append(reply_dbus, &iter_dbus);
	return true;
}

Php::Value Ldbus::message_new_method_call(Php::Parameters &params) {
	const char *bus = params[0];
	const char *object = params[1];
	const char *interface = params[2];
	const char *method = params[3];
	reply_dbus = dbus_message_new_method_call(bus, object, interface, method); // message_dbus ++++++++++++++++++++
    if (reply_dbus == NULL) {
        fprintf(stderr, "php-ldbus: Error creating dbus method.\n");
        return false;
    }
return true;
}


Php::Value Ldbus::test_function(Php::Parameters &params) {
	int function_value = (int)params[0];
	const char *function_name = params[1];
	Php::call(function_name, function_value);
	return true;
}

// ================================================================================================   
extern "C" {
    PHPCPP_EXPORT void *get_module() 
    {
        static Php::Extension extension("php-ldbus", "1.0");
        Php::Class<Ldbus> ldbus("Ldbus");
        ldbus.method<&Ldbus::__construct>("__construct");
        ldbus.method<&Ldbus::bus_request_name>("bus_request_name");
        ldbus.method<&Ldbus::connection_pop_message>("connection_pop_message");
        ldbus.method<&Ldbus::connection_send>("connection_send");
        ldbus.method<&Ldbus::connection_send_with_reply_and_block>("connection_send_with_reply_and_block");
        ldbus.method<&Ldbus::connection_flush>("connection_flush");
        ldbus.method<&Ldbus::message_get_path>("message_get_path");
        ldbus.method<&Ldbus::message_get_destination>("message_get_destination");
        ldbus.method<&Ldbus::message_get_type>("message_get_type");
        ldbus.method<&Ldbus::message_get_interface>("message_get_interface");
        ldbus.method<&Ldbus::message_get_member>("message_get_member");
        ldbus.method<&Ldbus::message_get_parameters>("message_get_parameters");
        ldbus.method<&Ldbus::message_is_method_call>("message_is_method_call");
        ldbus.method<&Ldbus::message_new_method_return>("message_new_method_return");
        ldbus.method<&Ldbus::message_iter_init_append>("message_iter_init_append");
        ldbus.method<&Ldbus::message_iter_append_basic>("message_iter_append_basic");
        ldbus.method<&Ldbus::message_new_method_call>("message_new_method_call");
        ldbus.method<&Ldbus::test_function>("test_function");
        extension.add(std::move(ldbus));
        extension.add(Php::Constant("DBUS_TYPE_STRING", DBUS_TYPE_STRING));
        extension.add(Php::Constant("DBUS_TYPE_UINT32", DBUS_TYPE_UINT32));
        extension.add(Php::Constant("DBUS_BUS_SESSION", DBUS_BUS_SESSION));
        extension.add(Php::Constant("DBUS_BUS_SYSTEM", DBUS_BUS_SYSTEM));
        return extension;
    }
}
