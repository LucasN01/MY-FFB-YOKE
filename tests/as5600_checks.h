#pragma once
#include "../As5600Position.h"

static void as5600Checks() {
    using namespace YokeAs5600;
    const int h=YOKE_AS5600_HYSTERESIS_COUNTS;
    // 1. Registro STATUS y mascara de los 12 bits de RAW ANGLE.
    const Frame f=decode(0x27,0xFF,0xFF);
    CHECK(f.raw==4095 && f.status==StatusMD);
    CHECK(decode(0xFF,0,0).status==(StatusMD|StatusML|StatusMH));
    // 2. Diferencias modulares, incluida la media vuelta.
    CHECK(wrapDelta(0)==0 && wrapDelta(2047)==2047);
    CHECK(wrapDelta(2048)==-2048 && wrapDelta(-2049)==2047);
    CHECK(wrapDelta(4095)==-1 && wrapDelta(-4095)==1);
    // 3. Cruce del cero en ambos sentidos, descontando la banda movil.
    Axis crossing(-1,1000,1000,1); crossing.start(0);
    crossing.update(true,4090,0); CHECK(crossing.counts()==0 && crossing.healthy());
    crossing.update(true,5,2); CHECK(crossing.counts()==11-h && crossing.raw()==5);
    crossing.update(true,4000,4); CHECK(crossing.counts()==-90+h);
    // 4. Acumulacion de varias vueltas y recorte de posicion.
    Axis turns(-1,5000,5000,1); turns.start(0); turns.update(true,0,0);
    for(int i=1;i<=6;++i) {
        turns.update(true,uint16_t((i*1000)%4096),uint32_t(i*2));
        CHECK(turns.counts()==i*1000-h && turns.healthy());
    }
    CHECK(turns.counts()>4096 && near(turns.position(),1));
    // 5. Cada primera lectura absoluta conserva accum sin descontar H.
    Axis absolute(3000,1000,1000,1), negative(3000,1000,1000,1), wrapped(100,1000,1000,1);
    absolute.update(true,3500,0); CHECK(near(absolute.position(),.5f));
    negative.update(true,2600,0); CHECK(near(negative.position(),-.4f));
    wrapped.update(true,4000,0); CHECK(near(wrapped.position(),-.196f));
    absolute.update(true,2600,2); CHECK(near(absolute.position(),float(-400+h)/1000));
    // 6. Direccion y escalas asimetricas aplicadas a las cuentas firmadas.
    Axis reverse(3000,1000,2000,-1);
    reverse.update(true,3500,0); CHECK(reverse.counts()==-500 && near(reverse.position(),-.5f));
    reverse.update(true,2600,2); CHECK(near(reverse.position(),float(400-h)/2000));
    // 7. Ruido dentro de la banda y avance al superarla.
    Axis noise(-1,1000,1000,1); noise.update(true,2000,0);
    noise.update(true,uint16_t(2000+h),2); CHECK(noise.counts()==0);
    noise.update(true,uint16_t(2000-h),4); CHECK(noise.counts()==0);
    noise.update(true,uint16_t(2000+h+1),6); CHECK(noise.counts()==1);
    // 8. Umbrales de falla y recuperacion; una mala reinicia las buenas.
    Axis faults(2000,1000,1000,1); faults.update(true,2500,0);
    for(int i=1;i<YOKE_AS5600_FAULT_CYCLES;++i) {
        faults.update(false,0,uint32_t(i*2)); CHECK(faults.healthy());
    }
    faults.update(false,0,YOKE_AS5600_FAULT_CYCLES*2);
    CHECK(!faults.healthy() && near(faults.position(),.5f) && faults.counts()==500 && faults.raw()==2500);
    for(int i=1;i<YOKE_AS5600_RECOVER_CYCLES;++i) {
        faults.update(true,2500,uint32_t(100+i*2)); CHECK(!faults.healthy());
    }
    faults.update(false,0,1200); CHECK(!faults.healthy());
    for(int i=1;i<YOKE_AS5600_RECOVER_CYCLES;++i) {
        faults.update(true,2500,uint32_t(1200+i*2)); CHECK(!faults.healthy());
    }
    faults.update(true,2500,1200+YOKE_AS5600_RECOVER_CYCLES*2); CHECK(faults.healthy());
    // 9. Una falla corta no pierde el cruce de cero.
    Axis shortGap(-1,1000,1000,1); shortGap.update(true,4000,0);
    shortGap.update(false,0,2); shortGap.update(true,50,YOKE_AS5600_MAX_GAP_MS-1);
    CHECK(!shortGap.locked() && shortGap.healthy() && shortGap.counts()==146-h);
    // 10. Hueco largo con o sin errores; el bloqueo es permanente.
    for(int bad=0;bad<2;++bad) {
        Axis gap(-1,1000,1000,1); gap.update(true,2000,0); gap.update(true,2100,2);
        const long held=gap.counts(); const float pos=gap.position();
        if(bad) gap.update(false,0,4);
        gap.update(true,2200,2+YOKE_AS5600_MAX_GAP_MS+1);
        CHECK(gap.locked() && !gap.healthy() && gap.counts()==held && near(gap.position(),pos));
        for(int i=0;i<1000;++i) gap.update(true,2000,uint32_t(200+i*2));
        CHECK(gap.locked() && !gap.healthy() && gap.counts()==held && near(gap.position(),pos));
    }
    // 11. Centro absoluto: resincronizacion incluso tras una falla larga.
    Axis resync(2000,1000,1000,1); resync.update(true,2500,0);
    for(int i=0;i<YOKE_AS5600_FAULT_CYCLES;++i) resync.update(false,0,uint32_t(i*2+2));
    for(int i=0;i<YOKE_AS5600_RECOVER_CYCLES;++i)
        resync.update(true,1500,uint32_t(YOKE_AS5600_MAX_GAP_MS+100+i*2));
    CHECK(!resync.locked() && resync.healthy() && resync.counts()==-500+h);
    // 12. Ninguna posicion valida antes de inicializar.
    Axis waiting(-1,1000,1000,1); waiting.start(0);
    CHECK(!waiting.healthy() && !waiting.locked() && near(waiting.position(),0) && waiting.counts()==0);
    waiting.update(false,4095,2); CHECK(!waiting.healthy() && near(waiting.position(),0));
    // 13. Resta unsigned al desbordar millis(): 32 ms.
    Axis rollover(-1,1000,1000,1); rollover.start(0xFFFFFFF0u);
    rollover.update(true,2000,0xFFFFFFF0u); rollover.update(true,2100,0x10u);
    CHECK(!rollover.locked() && rollover.healthy() && rollover.counts()==100-h);
    // 14. La excusa de flash se consume en la siguiente lectura buena.
    Axis flash(-1,1000,1000,1); flash.update(true,2000,0); flash.excuseGap();
    flash.update(false,0,2); flash.update(true,2100,400);
    CHECK(!flash.locked() && flash.healthy() && flash.counts()==100-h);
    flash.update(true,2200,400+YOKE_AS5600_MAX_GAP_MS+1); CHECK(flash.locked());
    Axis consume(-1,1000,1000,1); consume.update(true,2000,0); consume.excuseGap();
    consume.update(true,2000,2); consume.update(true,2000,3+YOKE_AS5600_MAX_GAP_MS);
    CHECK(consume.locked());
    // 15. Ventana de arranque desde start; centro absoluto no la necesita.
    Axis late(-1,1000,1000,1); late.start(100);
    late.update(true,2000,100+YOKE_AS5600_START_WINDOW_MS+1);
    CHECK(late.locked() && !late.healthy() && late.counts()==0);
    Axis timely(-1,1000,1000,1); timely.start(100);
    timely.update(true,2000,100+YOKE_AS5600_START_WINDOW_MS); CHECK(timely.healthy());
    Axis lateAbsolute(2000,1000,1000,1); lateAbsolute.start(100);
    lateAbsolute.update(true,2500,101+YOKE_AS5600_START_WINDOW_MS); CHECK(lateAbsolute.healthy());
    // 16. Sobrerrecorrido: ambos sentidos, conserva la ultima posicion.
    for(int sign : {-1,1}) {
        Axis travel(-1,1000,1000,1); travel.update(true,2000,0);
        travel.update(true,uint16_t(2000+sign*1000),2);
        const long held=travel.counts(); const float pos=travel.position();
        const int excess=int(YOKE_AS5600_OVERTRAVEL*1000)+1;
        travel.update(true,uint16_t(2000+sign*excess),4);
        CHECK(travel.locked() && !travel.healthy() && travel.counts()==held && near(travel.position(),pos));
        Axis unrestricted(2000,1000,1000,1);
        unrestricted.update(true,uint16_t(2000+sign*excess),0);
        CHECK(!unrestricted.locked() && unrestricted.healthy() && near(unrestricted.position(),float(sign)));
    }
}
