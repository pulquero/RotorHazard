#include <ArduinoUnitTests.h>
#include <Godmode.h>
#include "../rx.h"

unittest(bitbang_powerUp)
{
    BitBangSPI spi;
    RxModule rx;
    rx.init(&spi, 0, 0);
    rx.powerUp();
    assertFalse(rx.isPoweredDown());
}

unittest(bitbang_powerDown)
{
    BitBangSPI spi;
    RxModule rx;
    rx.init(&spi, 0, 0);
    rx.powerDown();
    assertTrue(rx.isPoweredDown());
}

unittest_main()
