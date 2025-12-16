#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <msgpack.hpp>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include "EEGData.h"
#include "WristbandData.h"

// @formatter:off
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct convert<QByteArray> {
    msgpack::object const& operator()(msgpack::object const& o, QByteArray& v) const {
        if (o.type != type::BIN) throw type_error();
        v = QByteArray(o.via.bin.ptr, o.via.bin.size);
        return o;
    }
};

template<>
struct pack<QByteArray> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, QByteArray const& v) const {
        o.pack_bin(v.size());
        o.pack_bin_body(v.constData(), v.size());
        return o;
    }
};

template<>
struct object_with_zone<QByteArray> {
    void operator()(msgpack::object::with_zone& o, QByteArray const& v) const {
        char* ptr = static_cast<char*>(o.zone.allocate_align(v.size(), MSGPACK_ZONE_ALIGNOF(char)));
        std::memcpy(ptr, v.constData(), v.size());
        o.type = type::BIN;
        o.via.bin.ptr = ptr;
        o.via.bin.size = v.size();
    }
};

template<>
struct convert<QString> {
    msgpack::object const& operator()(msgpack::object const& o, QString& v) const {
        if (o.type != type::STR) throw type_error();
        v = QString::fromUtf8(o.via.str.ptr, o.via.str.size);
        return o;
    }
};

template<>
struct pack<QString> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, QString const& v) const {
        const QByteArray utf8 = v.toUtf8();
        o.pack_str(utf8.size());
        o.pack_str_body(utf8.constData(), utf8.size());
        return o;
    }
};

template<>
struct object_with_zone<QString> {
    void operator()(msgpack::object::with_zone& o, QString const& v) const {
        const QByteArray utf8 = v.toUtf8();
        char* ptr = static_cast<char*>(o.zone.allocate_align(utf8.size(), MSGPACK_ZONE_ALIGNOF(char)));
        std::memcpy(ptr, utf8.constData(), utf8.size());
        o.type = type::STR;
        o.via.str.ptr = ptr;
        o.via.str.size = utf8.size();
    }
};

template <typename T>
struct convert<QList<T>> {
    msgpack::object const& operator()(msgpack::object const& o, QList<T>& v) const {
        if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
        v.reserve(o.via.array.size);
        for (uint32_t i = 0; i < o.via.array.size; ++i) {
            T item = o.via.array.ptr[i].as<T>();
            v.append(std::move(item));
        }
        return o;
    }
};

template <typename T>
struct pack<QList<T>> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, QList<T> const& v) const {
        o.pack_array(v.size());
        for (const auto& e : v) {
            o.pack(e);
        }
        return o;
    }
};

template <typename T>
struct object_with_zone<QList<T>> {
    void operator()(msgpack::object::with_zone& o, QList<T> const& v) const {
        o.type = msgpack::type::ARRAY;
        o.via.array.size = v.size();
        o.via.array.ptr = static_cast<msgpack::object*>(
            o.zone.allocate_align(sizeof(msgpack::object) * v.size(), MSGPACK_ZONE_ALIGNOF(msgpack::object)));

        for (uint32_t i = 0; i < v.size(); ++i) {
            o.via.array.ptr[i] = msgpack::object(v[i], o.zone);
        }
    }
};

template<>
struct convert<EEGSensorData> {
    msgpack::object const& operator()(msgpack::object const& o, EEGSensorData& v) const {
        if (o.type != type::ARRAY) throw type_error();
        if (o.via.array.size != 4) throw type_error();

        v.timestamp = o.via.array.ptr[0].as<qint64>();
        v.adcStatus = o.via.array.ptr[1].as<QByteArray>();
        v.channelData = o.via.array.ptr[2].as<std::array<float, 24>>();
        v.trigger = o.via.array.ptr[3].as<float>();
        return o;
    }
};

template<>
struct pack<EEGSensorData> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, EEGSensorData const& v) const {
        o.pack_array(4);
        o.pack(v.timestamp);
        o.pack(v.adcStatus);
        o.pack(v.channelData);
        o.pack(v.trigger);
        return o;
    }
};

template<>
struct object_with_zone<EEGSensorData> {
    void operator()(msgpack::object::with_zone& o, EEGSensorData const& v) const {
        o.type = msgpack::type::ARRAY;
        o.via.array.size = 4;
        o.via.array.ptr = static_cast<msgpack::object*>(
            o.zone.allocate_align(sizeof(msgpack::object) * 4, MSGPACK_ZONE_ALIGNOF(msgpack::object)));

        o.via.array.ptr[0] = msgpack::object(v.timestamp, o.zone);
        o.via.array.ptr[1] = msgpack::object(v.adcStatus, o.zone);
        o.via.array.ptr[2] = msgpack::object(v.channelData, o.zone);
        o.via.array.ptr[3] = msgpack::object(v.trigger, o.zone);
    }
};

template<>
struct convert<EEGEventData> {
    msgpack::object const& operator()(msgpack::object const& o, EEGEventData& v) const {
        if (o.type != type::ARRAY) throw type_error();
        if (o.via.array.size != 3) throw type_error();

        v.timestamp = o.via.array.ptr[0].as<qint64>();
        v.code = o.via.array.ptr[1].as<quint64>();

        const auto& msg_obj = o.via.array.ptr[2];

        switch (v.code) {
            case 1: // Greeting/Version, {'version': '...'}
            case 9: { // Sensor Map, {'sensorMap': '...'}
                if (msg_obj.type != type::MAP) throw type_error();
                if (msg_obj.via.map.size != 1) throw type_error();
                QString key_str = msg_obj.via.map.ptr[0].key.as<QString>();
                QString val_str = msg_obj.via.map.ptr[0].val.as<QString>();
                v.message = {
                    {key_str, val_str}
                };
                break;
            }
            case 10: { // Data Rate, {'mainsFreq': 50, 'sampleFreq': 300}
                if (msg_obj.type != type::MAP) throw type_error();
                if (msg_obj.via.map.size != 2) throw type_error();
                const auto& map_obj = msg_obj.via.map;
                v.message = QVariantMap();
                for (uint32_t i = 0; i < 2; ++i) {
                    QString key_str = map_obj.ptr[i].key.as<QString>();
                    qint64 val = map_obj.ptr[i].val.as<qint64>();
                    v.message.insert(key_str,val);
                }
                break;
            }
            default:
                v.message = QVariantMap();
                break;
        }
        return o;
    }
};

template<>
struct pack<EEGEventData> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, EEGEventData const& v) const {
        o.pack_array(3);
        o.pack(v.timestamp);
        o.pack(v.code);

        switch (v.code) {
            case 1:  // Greeting/Version, {'version': '...'}
            case 9:  // Sensor Map, {'sensorMap': '...'}
            case 10: { // Data Rate, {'mainsFreq': 50, 'sampleFreq': 300}
                o.pack_map(v.message.size());
                for (auto it = v.message.cbegin(); it != v.message.cend(); ++it) {
                    // 打包键
                    o.pack(it.key());
                    // 打包值
                    const QVariant& val = it.value();
                    if (v.code == 10)
                        o.pack(val.toInt());
                    else
                        o.pack(val.toString());
                }
                break;
            }
            default:
                o.pack_nil();
        }
        return o;
    }
};

template<>
struct object_with_zone<EEGEventData> {
    template<typename T>
    static T* allocate(msgpack::zone& zone, const size_t num) {
        const size_t size = num * sizeof(T);
        return static_cast<T*>(zone.allocate_align(size, MSGPACK_ZONE_ALIGNOF(T)));
    }

    void operator()(msgpack::object::with_zone& o, EEGEventData const& v) const {
        o.type = msgpack::type::ARRAY;
        o.via.array.size = 3;
        o.via.array.ptr = allocate<msgpack::object>(o.zone, 3);

        o.via.array.ptr[0] = msgpack::object(v.timestamp, o.zone);
        o.via.array.ptr[1] = msgpack::object(v.code, o.zone);

        auto& msg_ptr = o.via.array.ptr[2];

        switch (v.code) {
            case 1:  // Greeting/Version, {'version': '...'}
            case 9:  // Sensor Map, {'sensorMap': '...'}
            case 10: { // Data Rate, {'mainsFreq': 50, 'sampleFreq': 300}
                auto* map = allocate<msgpack::object_kv>(o.zone, v.message.size());
                int idx = 0;
                for (auto it = v.message.cbegin(); it != v.message.cend(); ++it, ++idx) {
                    // 打包键
                    map[idx].key = msgpack::object(it.key(), o.zone);
                    // 打包值
                    if (v.code == 10)
                        map[idx].val = msgpack::object(it.value().toInt(), o.zone);
                    else
                        map[idx].val = msgpack::object(it.value().toString(), o.zone);
                }
                msg_ptr.type = msgpack::type::MAP;
                msg_ptr.via.map.size = v.message.size();
                msg_ptr.via.map.ptr = map;
                break;
            }
            default:
                msg_ptr.type = msgpack::type::NIL;
        }
    }
};

template<>
struct convert<EEGPacket> {
    msgpack::object const& operator()(msgpack::object const& o, EEGPacket& v) const {
        o.convert(v.data);
        return o;
    }
};

template<>
struct pack<EEGPacket> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, EEGPacket const& v) const {
        o.pack(v.data);
        return o;
    }
};

template<>
struct object_with_zone<EEGPacket> {
    void operator()(msgpack::object::with_zone& o, EEGPacket const& v) const {
        const msgpack::object tmp(v.data, o.zone);
        o.type = tmp.type;
        o.via = tmp.via;
    }
};

template<>
struct convert<PulseWaveValue> {
    msgpack::object const& operator()(msgpack::object const& o, PulseWaveValue& v) const {
        if (o.type != type::ARRAY) throw type_error();
        if (o.via.array.size != 2) throw type_error();

        v = PulseWaveValue(
            o.via.array.ptr[0].as<qint64>(),
            o.via.array.ptr[1].as<qreal>()
        );
        return o;
    }
};

template<>
struct pack<PulseWaveValue> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, PulseWaveValue const& v) const {
        o.pack_array(2);
        o.pack(v.raw);
        o.pack(v.filtered);
        return o;
    }
};

template<>
struct object_with_zone<PulseWaveValue> {
    void operator()(msgpack::object::with_zone& o, PulseWaveValue const& v) const {
        o.type = msgpack::type::ARRAY;
        o.via.array.size = 2;
        o.via.array.ptr = static_cast<msgpack::object*>(
            o.zone.allocate_align(sizeof(msgpack::object) * 2, MSGPACK_ZONE_ALIGNOF(msgpack::object)));

        o.via.array.ptr[0] = msgpack::object(v.raw, o.zone);
        o.via.array.ptr[1] = msgpack::object(v.filtered, o.zone);
    }
};

template<>
struct convert<AccValue> {
    msgpack::object const& operator()(msgpack::object const& o, AccValue& v) const {
        if (o.type != type::ARRAY) throw type_error();
        if (o.via.array.size != 3) throw type_error();

        v = AccValue(
            o.via.array.ptr[0].as<qreal>(),
            o.via.array.ptr[1].as<qreal>(),
            o.via.array.ptr[2].as<qreal>()
        );
        return o;
    }
};

template<>
struct pack<AccValue> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, AccValue const& v) const {
        o.pack_array(3);
        o.pack(v.x);
        o.pack(v.y);
        o.pack(v.z);
        return o;
    }
};

template<>
struct object_with_zone<AccValue> {
    void operator()(msgpack::object::with_zone& o, AccValue const& v) const {
        o.type = msgpack::type::ARRAY;
        o.via.array.size = 3;
        o.via.array.ptr = static_cast<msgpack::object*>(
            o.zone.allocate_align(sizeof(msgpack::object) * 3, MSGPACK_ZONE_ALIGNOF(msgpack::object)));

        o.via.array.ptr[0] = msgpack::object(v.x, o.zone);
        o.via.array.ptr[1] = msgpack::object(v.y, o.zone);
        o.via.array.ptr[2] = msgpack::object(v.z, o.zone);
    }
};

template<>
struct convert<WristbandPacket> {
    msgpack::object const& operator()(msgpack::object const& o, WristbandPacket& v) const {
        if (o.type != type::ARRAY) throw type_error();
        if (o.via.array.size != 5) throw type_error();

        v = WristbandPacket(
            o.via.array.ptr[0].as<qint64>(),
            o.via.array.ptr[1].as<HrValue>(),
            o.via.array.ptr[2].as<QList<PulseWaveValue> >(),
            o.via.array.ptr[3].as<QList<GsrValue> >(),
            o.via.array.ptr[4].as<QList<AccValue> >()
        );
        return o;
    }
};

template<>
struct pack<WristbandPacket> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, WristbandPacket const& v) const {
        o.pack_array(5);
        o.pack(v.timestamp());
        o.pack(v.hr());
        o.pack(v.pulseWaveList());
        o.pack(v.gsrList());
        o.pack(v.accList());
        return o;
    }
};

template<>
struct object_with_zone<WristbandPacket> {
    void operator()(msgpack::object::with_zone& o, WristbandPacket const& v) const {
        o.type = msgpack::type::ARRAY;
        o.via.array.size = 5;
        o.via.array.ptr = static_cast<msgpack::object*>(
            o.zone.allocate_align(sizeof(msgpack::object) * 5, MSGPACK_ZONE_ALIGNOF(msgpack::object)));

        o.via.array.ptr[0] = msgpack::object(v.timestamp(), o.zone);
        o.via.array.ptr[1] = msgpack::object(v.hr(), o.zone);
        o.via.array.ptr[2] = msgpack::object(v.pulseWaveList(), o.zone);
        o.via.array.ptr[3] = msgpack::object(v.gsrList(), o.zone);
        o.via.array.ptr[4] = msgpack::object(v.accList(), o.zone);
    }
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
// @formatter:on

#endif //SERIALIZE_H
