Welcome to PHP-LDBUS binding.

This is binding of d-bus library, which allows to use it directly from PHP code. Ldbus means low-level. This means this library can be used without GIO or any Gnome dependencies - only directly, only hardcore.

Installation.

This library means you already have PHP installed, and framework PHP-CPP installed. If not: https://www.php-cpp.com/documentation/install
```
git clone https://github.com/ntfs1984/php-ldbus;cd php-ldbus;make
```
After then you should copy compiled .so library to any location and put "`extension = path/to/this/library.so`" to your php.ini file

Shortly usage:

a) Execute some method with string parameter (currently only string and int are supported)
```php
<?php
$dbus = new Ldbus(DBUS_BUS_SESSION); // Make new instance. After creating instance - you now connected to d-bus
$dbus->message_new_method_call("org.supreme.Session","/org/supreme/Session","org.supreme.Session","Execute"); // Prepare to call $bus, $object, $interface, $method
$dbus->message_iter_init_append(); // Prepare to fill data which we will send in method call
$dbus->message_iter_append_basic(DBUS_TYPE_STRING, "galculator"); // So, we will call method "Execute" with parameter "galculator"
$dbus->connection_send(); // Sending prepared message
?>
```

b) Create own d-bus service

```php
<?php
$dbus = new Ldbus(DBUS_BUS_SESSION); // Make new instance. After creating instance - you now connected to d-bus
$introspection_xml = "<node><interface name=\"org.freedesktop.DBus.Properties\"><method name=\"Get\"><arg type=\"s\" name=\"interface_name\" direction=\"in\"/><arg type=\"s\" name=\"property_name\" direction=\"in\"/><arg type=\"v\" name=\"value\" direction=\"out\"/></method><method name=\"GetAll\"><arg type=\"s\" name=\"interface_name\" direction=\"in\"/><arg type=\"a{sv}\" name=\"properties\" direction=\"out\"/></method><method name=\"Set\"><arg type=\"s\" name=\"interface_name\" direction=\"in\"/><arg type=\"s\" name=\"property_name\" direction=\"in\"/><arg type=\"v\" name=\"value\" direction=\"in\"/></method><signal name=\"PropertiesChanged\"><arg type=\"s\" name=\"interface_name\"/><arg type=\"a{sv}\" name=\"changed_properties\"/><arg type=\"as\" name=\"invalidated_properties\"/></signal></interface><interface name=\"org.freedesktop.DBus.Introspectable\"><method name=\"Introspect\"><arg type=\"s\" name=\"xml_data\" direction=\"out\"/></method></interface><interface name=\"org.freedesktop.DBus.Peer\"><method name=\"Ping\"/><method name=\"GetMachineId\"><arg type=\"s\" name=\"machine_uuid\" direction=\"out\"/></method></interface><interface name=\"org.xfce.Notifyd\"><method name=\"Quit\"/></interface><interface name=\"org.freedesktop.Notifications\"><method name=\"GetCapabilities\"><arg type=\"as\" name=\"capabilities\" direction=\"out\"/></method><method name=\"Notify\"><arg type=\"s\" name=\"app_name\" direction=\"in\"/><arg type=\"u\" name=\"replaces_id\" direction=\"in\"/><arg type=\"s\" name=\"app_icon\" direction=\"in\"/><arg type=\"s\" name=\"summary\" direction=\"in\"/><arg type=\"s\" name=\"body\" direction=\"in\"/><arg type=\"as\" name=\"actions\" direction=\"in\"/><arg type=\"a{sv}\" name=\"hints\" direction=\"in\"/><arg type=\"i\" name=\"expire_timeout\" direction=\"in\"/><arg type=\"u\" name=\"id\" direction=\"out\"/></method><method name=\"CloseNotification\"><arg type=\"u\" name=\"id\" direction=\"in\"/></method><method name=\"GetServerInformation\"><arg type=\"s\" name=\"name\" direction=\"out\"/><arg type=\"s\" name=\"vendor\" direction=\"out\"/><arg type=\"s\" name=\"version\" direction=\"out\"/><arg type=\"s\" name=\"spec_version\" direction=\"out\"/></method><signal name=\"NotificationClosed\"><arg type=\"u\" name=\"id\"/><arg type=\"u\" name=\"reason\"/></signal><signal name=\"ActionInvoked\"><arg type=\"u\" name=\"id\"/><arg type=\"s\" name=\"action_key\"/></signal></interface></node>";
$dbus->bus_request_name("any.busname.com");
while(true) {
	if ($dbus->connection_pop_message()) {
		$path = $dbus->message_get_path();
		$dest = $dbus->message_get_destination();
		$type = $dbus->message_get_type();
		$intrf = $dbus->message_get_interface();
		$member = $dbus->message_get_member();
		$parameters = $dbus->message_get_parameters();
		echo "Message path: $path; destination: $dest; type: $type; interface: $intrf; member: $member; parameters: ";print_r($parameters);echo "\n";
		
		if ($member == "Introspect") {
			$dbus->message_new_method_return();
			$dbus->message_iter_init_append();
			$dbus->message_iter_append_basic(DBUS_TYPE_STRING,"$introspection_xml");
			$dbus->connection_send();
			$dbus->connection_flush();
			
		}
	}
}
```
To explore and usage your d-bus service, it should be "introspectable". If easy, when anyone wants to explore your services, it's simply executing method called "Introspect" on a way like we did in previous example.
Your service must respond to this query within XML text, which contains special markup of your service, it's methods, properties and signals. If your service will ignore this query - dbus-viewer or any other application could report about error.
Introspection example got from notification daemon.

Using example variables, you might catch any request to any your service, and respond to query as you wish.

Urgent: your PHP application must be designed to periodically check for incoming messages. These operations are sync, running in main thread, so your application will be blocked for some small time.

c) Watch for signal
```php
<?php
$dbus = new Ldbus(DBUS_BUS_SESSION); // Make new instance. After creating instance - you now connected to d-bus
$rule = "type='signal',interface='org.freedesktop.Notifications',member='NotificationClosed'"; // We will watch for signal called NotificationClosed - when we are closing any popup notification
$dbus->add_match($rule);
while(true) {
	if ($dbus->connection_pop_message()) {
		$path = $dbus->message_get_path();
		$dest = $dbus->message_get_destination();
		$type = $dbus->message_get_type();
		$intrf = $dbus->message_get_interface();
		$member = $dbus->message_get_member();
		$parameters = $dbus->message_get_parameters();
		echo "Message path: $path; destination: $dest; type: $type; interface: $intrf; member: $member; parameters: ";print_r($parameters);echo "\n";
	}
}
$dbus->remove_match($rule);
```
As you see, we are using almost the same functions like at previous example, but this time we telling system, that we want watch only some messages: for signal, and for specific interface.
Notice: after you done, you should remove_match, to reset filters.
