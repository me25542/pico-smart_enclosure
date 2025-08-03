# Contributing

Thank you for your interest in contributing! This project targets the **Raspberry Pi Pico** using **PlatformIO** and the **Arduino-Pico core**. Clean, readable, and maintainable code is welcome. The following guidelines will help keep the codebase consistent.

## Getting Started

- The only officially supported environment is [PlatformIO](https://platformio.org).
  - While it may be possible to adapt the project for use with the Arduino IDE, this is **not** officially supported and may stop working in the future.
  - It's also not ideal if you want to contribute changes.
- This project uses the excellent [Arduino-Pico core](https://github.com/earlephilhower/arduino-pico) by [Earle F. Philhower](https://github.com/earlephilhower) and other contributors (released under the GNU Lesser General Public License v2.1).
  - Follow the [setup instructions](https://github.com/earlephilhower/arduino-pico/blob/master/docs/platformio.rst) found in its repository to configure PlatformIO correctly.
- Install all dependencies (see documentation).
- Clone this repository and open it with an editor that has PlatformIO installed.
- Build and test your changes on actual hardware when possible - this isn’t strictly required but is highly encouraged.
- Open an issue or discussion before starting any major changes.

## Code Style

The code doesn’t follow a strict linter, but please match the general style and feel of the codebase.

### Indentation and Whitespace

- **Use tabs**, not spaces, for indentation.
- Keep vertical whitespace clean:
  - **No more than one blank line** between statements or functions.
- Prefer **minimal indentation** when it improves readability.
  - If you're more than about three levels deep, consider whether the code could be restructured.

### Naming Conventions

- Use **camelCase** for most variables and function names:  
  `exampleVariable`, `doSomethingUseful()`.
- Prefix **class member variables** with the class name and an underscore:  
  `exampleClass_exampleMember`.
- **Avoid abbreviations** in names. Prioritize clarity over brevity.

### Header Files

- Always use `#pragma once` at the top of header files instead of include guards.

### Braces and Control Structures

Use this formatting style for `if`/`else` blocks:

```cpp
if (condition) {
	// do something

} else if (otherCondition) {
	// do something else

} else {
	// fallback
}
````

This style improves readability by visually separating control flow blocks.

### Comments

- **Single-line comments** should have a space after the `//`:
  `// good comment`
- **Inline comments** for variables can use `///<`:
  `int example ///< describes what this is for`
- **Functions** and important blocks should use **Doxygen-style** comments:

  ```cpp
  /**
   * @brief Brief description of what the function does.
   */
  void someFunction();
  ```

### Memory Allocation and Pointers

- Avoid raw pointers and manual memory allocation when possible.
- These features are not forbidden and can be useful, but use caution - they can introduce subtle bugs.
- In general, prefer simpler, allocation-free approaches when they achieve the same result without major compromise.

### General Formatting

- Write **clean, conventional C++**.
- Follow standard spacing and punctuation unless the project style says otherwise.
- You don’t need to be perfect - just aim to match the existing code style.

## Submitting Changes

1. Fork the repository.
2. Create a new branch: `git checkout -b feature/my-feature`.
3. Make your changes and commit them with clear messages.
4. Push to your fork and open a pull request (PR).

## Additional Notes

- If you're unsure about a formatting rule, **just do your best** - we can tidy things up during review.
- Large changes or refactors should be discussed in an issue first.
- Use common sense.
