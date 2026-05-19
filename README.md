# MQTT Sensor (STM32)

Tento projekt implementuje vestavný (embedded) systém IoT senzorové stanice postavené na mikrokontroléru řady STM32. Systém integruje barometrické a teplotní senzory, lokální grafický výstup na TFT displej, síťovou konektivitu realizovanou pomocí TCP/IP stacku LwIP s protokolem MQTT a rozhraní pro správu pomocí sériové linky (CLI).

## Hlavní funkcionalita

- **Sběr telemetrie:** Kontinuální měření atmosférického tlaku a teploty.
- **Lokální vizualizace:** Okamžité zobrazení IP adresy, stavu sítě a naměřených dat na barevném TFT displeji.
- **IoT Konektivita:** Periodické odesílání dat na MQTT broker pro další zpracování (např. Node-RED, Home Assistant).
- **Vzdálená správa:** Ovládání a diagnostika zařízení v reálném čase přes UART rozhraní.

---

## 1. Architektura softwaru a popis modulů

Projekt je striktně rozdělen do samostatných vrstev a modulů pro zajištění čisté a snadno udržovatelatelné struktury kódu:

- **`main.c`**
  - Zajišťuje kompletní inicializaci systému po resetu mikrokontroléru (konfigurace hodin, inicializace HAL vrstvy).
  - V hlavní nekonečné smyčce koordinuje periodické vyčítání senzorů, překreslování displeje a obsluhu síťového stacku LwIP.

- **`bmp180.c`**
  - Zajišťuje komunikaci s digitálním senzorem tlaku a teploty Bosch BMP180 přes rozhraní I2C.
  - Obsahuje rutiny pro načtení továrních kalibračních koeficientů z EEPROM paměti senzoru a kompenzační algoritmy pro výpočet reálné teploty (°C) a tlaku (Pa).

- **`ili9341.c`**
  - Ovladač barevného TFT displeje s řadičem ILI9341 běžící přes rychlé sériové rozhraní SPI.
  - Implementuje nízkoúrovňové grafické funkce (inicializace, vykreslení pixelu, plnění oblastí, vykreslování textových řetězců pomocí rastrových písem).

- **`mqtt_app.c`**
  - Integruje síťovou aplikaci s otevřeným TCP/IP stackem LwIP.
  - Zajišťuje navázání spojení s MQTT brokerem, periodicky publikuje naměřená data do specifických témat (topics) a implementuje callbacky pro příjem vzdálených příkazů.

- **`cli.c`**
  - Rozhraní příkazového řádku (Command Line Interface) běžící nad asynchronní sériovou linkou (UART).
  - Analyzuje přijaté textové řetězce z terminálu (např. PuTTY) a spouští příslušné interní funkce zařízení.

---

## 2. Hardwarová rozhraní a konfigurace periférií

Pro správnou funkci systému jsou periférie mikrokontroléru nakonfigurovány následovně:

| Sběrnice / Periférie | Cílové zařízení        | Konfigurace / Parametry                                       |
| :------------------- | :--------------------- | :------------------------------------------------------------ |
| **I2C1**             | Senzor BMP180          | Standard Mode (100 kHz), 7-bitová adresa zařízení             |
| **SPI1**             | Displej ILI9341        | Master mode, 8-bit, CPOL=0, CPHA=0, vysoká rychlost           |
| **USART1 / USART2**  | CLI Konzole / Ladění   | Rychlost 115200 baud, 8 datových bitů, 1 stop bit, bez parity |
| **RMII / MII**       | Ethernet PHY (LAN8720) | Propojení s LwIP stackem, automatická konfigurace DHCP        |

---

## 3. Specifikace CLI příkazů

Implementované uživatelské rozhraní konzole podporuje následující sadu příkazů pro diagnostiku a řízení stanice v reálném čase:

| Příkaz   | Argumenty    | Popis chování systému                                                                   |
| :------- | :----------- | :-------------------------------------------------------------------------------------- |
| `led`    | `on` / `off` | Rozsvítí nebo zhasne vestavěnou diagnostickou LED diodu na desce.                       |
| `reboot` | _žádné_      | Vyvolá okamžitý softwarový reset mikrokontroléru pomocí instrukce `NVIC_SystemReset()`. |
| `renew`  | _žádné_      | Vynutí uvolnění stávající IP adresy a odešle nový DHCP požadavek do sítě.               |
| `clear`  | _žádné_      | Odešle ANSI únikovou sekvenci pro vyčištění obrazovky sériového terminálu.              |

---

## 4. Instrukce k sestavení a spuštění

1. Otevřete projekt v prostředí **STM32CubeIDE**.
2. Ujistěte se, že soubor `.gitignore` správně filtruje složku `Debug/` a lokální nastavení `.settings/`.
3. Proveďte kompilaci projektu (**Build Project**).
4. Připojte vývojovou desku pomocí ST-Link programátoru.
5. Nahrajte firmware do mikrokontroléru (**Flash / Debug**).
6. Po spuštění můžete sledovat stav náběhu sítě a kalibrace senzorů na TFT displeji nebo se připojit k sériové lince s rychlostí `115200 baud` pro přístup k CLI rozhraní.
