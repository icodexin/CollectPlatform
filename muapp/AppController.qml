pragma Singleton
import QtQuick
import HuskarUI.Basic

QtObject {
    signal requestShowMessage(var msg)

    function showMessage(type: HusMessage.MessageType, message: string, duration=3000) {
        requestShowMessage({
            "type": type,
            "message": message,
            "duration": duration
        });
    }
}