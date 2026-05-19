# MQTT Sensor (STM32)

Tento projekt implementuje vestavný (embedded) systém IoT senzorové stanice postavené na mikrokontroléru řady STM32. Systém integruje barometrické a teplotní senzory, lokální grafický výstup na TFT displej, síťovou konektivitu realizovanou pomocí TCP/IP stacku LwIP s protokolem MQTT a rozhraní pro správu pomocí sériové linky (CLI).

## Hlavní funkcionalita

- **Sběr telemetrie:** Kontinuální měření atmosférického tlaku a teploty.
- **Lokální vizualizace:** Okamžité zobrazení IP adresy, stavu sítě a naměřených dat na barevném TFT displeji.
- **IoT Konektivita:** Periodické odesílání dat na MQTT broker pro další zpracování.
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

## 2. Specifikace CLI příkazů

Implementované uživatelské rozhraní konzole podporuje následující sadu příkazů pro diagnostiku, síťovou konfiguraci a řízení stanice v reálném čase:

| Příkaz    | Argumenty              | Popis chování systému                                                                                              |
| :-------- | :--------------------- | :----------------------------------------------------------------------------------------------------------------- |
| `help`    | _žádné_                | Vypíše nápovědu se seznamem všech dostupných příkazů.                                                              |
| `info`    | _žádné_                | Vypíše detailní stav sítě (stav připojení, IP, maska, brána).                                                      |
| `ip`      | _žádné_                | Zobrazí pouze aktuální IPv4 adresu zařízení.                                                                       |
| `uptime`  | _žádné_                | Zobrazí dobu běhu systému od posledního spuštění (hodiny, minuty, vteřiny).                                        |
| `last`    | _žádné_                | Vypíše historii MQTT (poslední odeslanou zprávu a poslední přijaté téma se zprávou).                               |
| `monitor` | `[téma]` _(volitelné)_ | Přihlásí k odběru dané téma (výchozí je `#`) a vypisuje příchozí zprávy do terminálu. Ukončuje se pomocí `Ctrl+C`. |
| `send`    | `<text>`               | Odešle zadaný textový řetězec (payload) jako MQTT zprávu.                                                          |
| `led`     | `on` / `off`           | Rozsvítí nebo zhasne vestavěnou modrou LED diodu na desce.                                                         |
| `renew`   | _žádné_                | Vynutí uvolnění stávající IP adresy, zastaví staré DHCP a odešle nový požadavek.                                   |
| `reboot`  | _žádné_                | Vyvolá okamžitý softwarový reset mikrokontroléru pomocí instrukce `NVIC_SystemReset()`.                            |
| `clear`   | _žádné_                | Vymaže obrazovku sériového terminálu (pomocí ANSI únikové sekvence).                                               |

---
