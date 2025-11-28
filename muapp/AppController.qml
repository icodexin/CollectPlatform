pragma Singleton
import QtQuick
import HuskarUI.Basic

QtObject {
    signal requestShowMessage(var msg)

    property QtObject msg: QtObject {
        function info(message: string, duration = 3000) {
            show(HusMessage.Type_Message, message, duration);
        }

        function warning(message: string, duration = 3000) {
            show(HusMessage.Type_Warning, message, duration);
        }

        function error(message: string, duration = 3000) {
            show(HusMessage.Type_Error, message, duration);
        }

        function success(message: string, duration = 3000) {
            show(HusMessage.Type_Success, message, duration);
        }

        function show(type: HusMessage.MessageType, message: string, duration = 3000) {
            requestShowMessage({
                "type": type,
                "message": message,
                "duration": duration
            })
        }
    }
}