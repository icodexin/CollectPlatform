#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <msgpack.hpp>
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
struct convert<PulseWaveData> {
    msgpack::object const& operator()(msgpack::object const& o, PulseWaveData& v) const {
        if (o.type != type::MAP) throw type_error();
        if (o.via.map.size != 2) throw type_error();

        for (uint32_t i = 0; i < o.via.map.size; ++i) {
            const auto& kv = o.via.map.ptr[i];
            if (kv.key.as<QString>() == "raw") {
                v.raw = kv.val.as<qint64>();
            } else if (kv.key.as<QString>() == "filtered") {
                v.filtered = kv.val.as<qreal>();
            }
        }
        return o;
    }
};

template<>
struct pack<PulseWaveData> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, PulseWaveData const& v) const {
        o.pack_map(2);
        o.pack("raw");
        o.pack(v.raw);
        o.pack("filtered");
        o.pack(v.filtered);
        return o;
    }
};

template<>
struct object_with_zone<PulseWaveData> {
    void operator()(msgpack::object::with_zone& o, PulseWaveData const& v) const {
        o.type = msgpack::type::MAP;
        o.via.map.size = 2;
        o.via.map.ptr = static_cast<msgpack::object_kv*>(
            o.zone.allocate_align(sizeof(msgpack::object_kv) * 2, MSGPACK_ZONE_ALIGNOF(msgpack::object_kv)));

        o.via.map.ptr[0].key = msgpack::object("raw", o.zone);
        o.via.map.ptr[0].val = msgpack::object(v.raw, o.zone);
        o.via.map.ptr[1].key = msgpack::object("filtered", o.zone);
        o.via.map.ptr[1].val = msgpack::object(v.filtered, o.zone);
    }
};

template<>
struct convert<AccData> {
    msgpack::object const& operator()(msgpack::object const& o, AccData& v) const {
        if (o.type != type::MAP) throw type_error();
        if (o.via.map.size != 3) throw type_error();

        for (uint32_t i = 0; i < o.via.map.size; ++i) {
            const auto& kv = o.via.map.ptr[i];
            if (kv.key.as<QString>() == "x") {
                v.x = kv.val.as<qreal>();
            } else if (kv.key.as<QString>() == "y") {
                v.y = kv.val.as<qreal>();
            } else if (kv.key.as<QString>() == "z") {
                v.z = kv.val.as<qreal>();
            }
        }
        return o;
    }
};

template<>
struct pack<AccData> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, AccData const& v) const {
        o.pack_map(3);
        o.pack("x");
        o.pack(v.x);
        o.pack("y");
        o.pack(v.y);
        o.pack("z");
        o.pack(v.z);
        return o;
    }
};

template<>
struct object_with_zone<AccData> {
    void operator()(msgpack::object::with_zone& o, AccData const& v) const {
        o.type = msgpack::type::MAP;
        o.via.map.size = 3;
        o.via.map.ptr = static_cast<msgpack::object_kv*>(
            o.zone.allocate_align(sizeof(msgpack::object_kv) * 3, MSGPACK_ZONE_ALIGNOF(msgpack::object_kv)));

        o.via.map.ptr[0].key = msgpack::object("x", o.zone);
        o.via.map.ptr[0].val = msgpack::object(v.x, o.zone);
        o.via.map.ptr[1].key = msgpack::object("y", o.zone);
        o.via.map.ptr[1].val = msgpack::object(v.y, o.zone);
        o.via.map.ptr[2].key = msgpack::object("z", o.zone);
        o.via.map.ptr[2].val = msgpack::object(v.z, o.zone);
    }
};

template<>
struct convert<WristbandData> {
    msgpack::object const& operator()(msgpack::object const& o, WristbandData& v) const {
        if (o.type != type::MAP) throw type_error();
        if (o.via.map.size != 5) throw type_error();

        for (uint32_t i = 0; i < o.via.map.size; ++i) {
            auto& [key, val] = o.via.map.ptr[i];
            QString key_str = key.as<QString>();
            if (key_str == "timestamp") {
                v.setTimestamp(val.as<qint64>());
            } else if (key_str == "hr") {
                v.setHr(val.as<qreal>());
            } else if (key_str == "pulseWaves") {
                v.setPulseWaves(val.as<QList<PulseWaveData>>());
            } else if (key_str == "gsrs") {
                v.setGsrs(val.as<QList<qreal>>());
            } else if (key_str == "accs") {
                v.setAccs(val.as<QList<AccData>>());
            }
        }
        return o;
    }
};

template<>
struct pack<WristbandData> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, WristbandData const& v) const {
        o.pack_map(5);
        o.pack("timestamp");
        o.pack(v.timestamp());
        o.pack("hr");
        o.pack(v.hr());
        o.pack("pulseWaves");
        o.pack(v.pulseWaves());
        o.pack("gsrs");
        o.pack(v.gsrs());
        o.pack("accs");
        o.pack(v.accs());
        return o;
    }
};

template<>
struct object_with_zone<WristbandData> {
    void operator()(msgpack::object::with_zone& o, WristbandData const& v) const {
        o.type = msgpack::type::MAP;
        o.via.map.size = 5;
        o.via.map.ptr = static_cast<msgpack::object_kv*>(
            o.zone.allocate_align(sizeof(msgpack::object_kv) * 5, MSGPACK_ZONE_ALIGNOF(msgpack::object_kv)));

        o.via.map.ptr[0].key = msgpack::object("timestamp", o.zone);
        o.via.map.ptr[0].val = msgpack::object(v.timestamp(), o.zone);
        o.via.map.ptr[1].key = msgpack::object("hr", o.zone);
        o.via.map.ptr[1].val = msgpack::object(v.hr(), o.zone);
        o.via.map.ptr[2].key = msgpack::object("pulseWaves", o.zone);
        o.via.map.ptr[2].val = msgpack::object(v.pulseWaves(), o.zone);
        o.via.map.ptr[3].key = msgpack::object("gsrs", o.zone);
        o.via.map.ptr[3].val = msgpack::object(v.gsrs(), o.zone);
        o.via.map.ptr[4].key = msgpack::object("accs", o.zone);
        o.via.map.ptr[4].val = msgpack::object(v.accs(), o.zone);
    }
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
// @formatter:on

#endif //SERIALIZE_H
