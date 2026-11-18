# BareMetal-KV11-Calculator
A four-function floating-point calculator written in bare-metal C for the NXP Kinetis KV11 (MKV11Z7). Input comes from a 4x4 matrix keypad and results appear on an 8x2 Newhaven NHD-0208AZ character LCD. It uses a 4-bit mode LCD interface and a keypad scanner written with register-level GPIO access, and the application logic is a four-state finite state machine (FSM).

## Demo

https://github.com/user-attachments/assets/98db290b-a6b0-4b0c-b461-8df513e1d696

## Hardware
| Part | Notes |
| :--- | :--- |
| **MCU** | NXP Kinetis KV11 (FRDM-KV11Z) |
| **Display** | 8x2 Newhaven NHD-0208AZ character LCD (4-bit mode, 3.3V logic) |
| **Input** | 4x4 matrix keypad |
| **Misc** | 10k potentiometer (LCD contrast), 2x 10Ω resistors (backlight current limiting) |

## Wiring

### LCD
| LCD pin | MCU / Power |
| :--- | :--- |
| D4 | PTD0 |
| D5 | PTD1 |
| D6 | PTD2 |
| D7 | PTD3 |
| E | PTD4 |
| RS | PTD5 |
| RW | PTA1 (held low for write) |
| V0 (Contrast) | 10k potentiometer wiper |
| A (Backlight +) | 3.3V via 2x 10Ω resistors |

### Keypad
| Signal | MCU pin | Direction |
| :--- | :--- | :--- |
| Row 0-3 (driven) | PTC0-PTC3 | Output |
| Col 0-3 (read) | PTC4-PTC7 | Input (internal pull-up) |

## Operation

**Keypad Layout:**

```
1  2  3  A (add)
4  5  6  B (subtract)
7  8  9  C (clear)
*  0  #  D (shift / 2nd function)
```

`*` (multiply)
`#` (equals)

Shift Layer (Press 'D' first)
* D -> D = Divide (`/`)
* D -> 0 = Decimal point (`.`)
* D -> * = Negative sign (`-`)

## How it behaves

Numbers are capped at seven digits. The first number is entered on the top line. Pressing an operator shifts the cursor to the second line for the second number. Pressing # (equals) clears the entire screen and prints the final floating-point result on the top left. Dividing by zero shows ERROR. Typing a new digit after a result automatically resets the state machine for a new calculation.

## Architecture

* **LCD Driver:** The driver is written for 4-bit mode. Each byte is split into two nibbles and sent one after the other. RS selects whether a byte is a command or character data, RW is held low because the display is write-only, and a pulse on Enable latches each nibble in.
* **Keypad Scanner:** The scanner drives one row low at a time and reads all four columns. The columns have internal pull-ups, so they read high until a key is pressed. A press connects that column to the low row and pulls it low, and the row and column together index into a character map to get the key.
* **State Machine:** 
* State 1: Parses keypad input to build the first floating-point value.
* State 2: Awaits a valid math operator, allowing the user to overwrite their selection if they make a mistake.
* State 3: Jumps to the second line of the LCD and builds the second floating-point value.
* State 4: Computes the mathematical result, clears the screen, prints the answer, and idles until a new calculation is initiated.

## Testing

| Test Case | Expected Output | Result |
| :--- | :--- | :--- |
| `12A34#` | `46` | Pass |
| `D*5B9#` | `-14` | Pass |
| `7*8#` | `56` | Pass |
| `9DD3#` | `3` | Pass |
| `5D05A2D02#` | `7.7` | Pass |
| `9DD0#` | `ERROR` | Pass |
| `99999999` | Caps at `9999999` | Pass |
| `5A3C` | Screen clears | Pass |

## Known limitations

- **Fixed Delays:** The LCD driver does not read the hardware busy flag, relying entirely on fixed for loop delays.
- **Polling:** Keypad scanning is polled continuously in the main while(1) loop rather than using GPIO interrupts.
- **Debouncing:** Only debounces on the initial key press, not on key release.
- **Chained Operations:** Does not support continuous math chaining (5 + 5 + 5). The user must press # to evaluate the expression before starting a new one.

## Build
Built with Keil µVision. Startup files from NXP; core calculator logic and keypad driver are custom.
Note: The LCD driver is not included in this repository.
