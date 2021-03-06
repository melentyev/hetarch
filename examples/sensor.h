#ifndef HETARCH_EXAMPLES_MSP_SENSOR_H
#define HETARCH_EXAMPLES_MSP_SENSOR_H 1

#include <cstdint>
#include <iostream>

#include <dsl/base.h>
#include <conn.h>

#include <memregmap.inc>

#include "example_utils.h"

using namespace hetarch;
using namespace hetarch::dsl;

#define ADC10BUSY              (0x0001)       /* ADC10 Busy */
#define ADC10SC                (0x0001)       /* ADC10 Start Conversion */
#define ADC10ENC               (0x0002)       /* ADC10 Enable Conversion */
#define ADC10ON                (0x0010)       /* ADC10 On/enable */

template<typename Platform, unsigned LoggerLength>
class Sensor {
    const uint16_t JA_COUNT = 6;
    Function<Platform, void> __conversationJAx() {
        Param<uint16_t> number;
        return MakeFunction<Platform, void>("conversationJAx", Params(number), Seq(
            ADC10CTL0.Assign(ADC10CTL0 & Const<uint16_t>(~ADC10ENC)),
            ADC10MCTL0.Assign(number),
            ADC10CTL0.Assign(ADC10CTL0 | ADC10ENC | ADC10SC),
            While((ADC10CTL1 & Const<uint16_t>(ADC10BUSY)) == 0x01, Void()),
            jax[number].Assign(ADC10MEM0), Void()
        ));
    }
    Function<Platform, void> __allJA() {
        Local<uint16_t> i;
        return MakeFunction<Platform, void>("allJA", Params(),
            For(i.Assign(0), i < JA_COUNT, ++i,
                conversationJAx(i)
            )
        );
    };
public:
    Logger<Platform, LoggerLength> &logger;
    Global<Platform, uint16_t[6]> jax;
    Function<Platform, void> conversationJAx;
    Function<Platform, void> allJA;


    Sensor(mem::MemMgr<Platform> &memMgr, cg::ICodeGen &codeGen,
                       conn::IConnection<Platform> &conn, Logger<Platform, LoggerLength> &_logger):
            logger(_logger),
            jax(memMgr),
            conversationJAx(__conversationJAx()),
            allJA(__allJA())
    {
        conversationJAx.compile(codeGen, memMgr);
        conversationJAx.Send(conn);

        allJA.compile(codeGen, memMgr);
        allJA.Send(conn);
    }
};

#endif