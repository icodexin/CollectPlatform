pragma Singleton
import QtQuick
import HuskarUI.Basic

QtObject {
    signal requestShowMessage(var msg)
    signal requestCloseMessage(string key)
    signal requestSetMessageProperty(string key, string property, var value)

    signal requestShowNotification(var msg)
    signal requestCloseNotification(string key)
    signal requestSetNotificationProperty(string key, string property, var value)

    property QtObject msg: QtObject {
        function info(message: string, duration = 3000, key = '') {
            show(HusMessage.Type_Message, message, duration, key);
        }

        function warning(message: string, duration = 3000, key = '') {
            show(HusMessage.Type_Warning, message, duration, key);
        }

        function error(message: string, duration = 3000, key = '') {
            show(HusMessage.Type_Error, message, duration, key);
        }

        function success(message: string, duration = 3000, key = '') {
            show(HusMessage.Type_Success, message, duration, key);
        }

        function show(type: HusMessage.MessageType, message: string, duration = 3000, key = '') {
            requestShowMessage({
                "key": key,
                "type": type,
                "message": message,
                "duration": duration
            })
        }

        function close(key: string) {
            requestCloseMessage(key);
        }

        function setProperty(key: string, prop: string, value: var ) {
            requestSetMessageProperty(key, prop, value);
        }
    }

    property QtObject notify: QtObject {
        function info(title: string, message: string, duration = 5000, key = '') {
            show(HusNotification.Type_Message, title, message, duration, key);
        }

        function warning(title: string, message: string, duration = 5000, key = '') {
            show(HusNotification.Type_Warning, title, message, duration, key);
        }

        function error(title: string, message: string, duration = 5000, key = '') {
            show(HusNotification.Type_Error, title, message, duration, key);
        }

        function success(title: string, message: string, duration = 5000, key = '') {
            show(HusNotification.Type_Success, title, message, duration, key);
        }

        function show(type: HusNotification.MessageType, title: string, message: string, duration = 5000, key = '') {
            requestShowNotification({
                "key": key,
                "type": type,
                "message": title,
                "description": message,
                "duration": duration
            })
        }

        function close(key: string) {
            requestCloseNotification(key);
        }

        function setProperty(key: string, prop: string, value: var ) {
            requestSetNotificationProperty(key, prop, value);
        }
    }
}
