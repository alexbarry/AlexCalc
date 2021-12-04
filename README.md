# AlexCalc

Try the web version here: https://alexbarry.github.io/AlexCalc/

Available on:
* **Web (desktop or mobile)**: https://alexbarry.github.io/AlexCalc/
* **Android**: [Google Play](https://play.google.com/store/apps/details?id=net.alexbarry.calc_android)
* **iOS**: No native app yet, but for now here's [how to add a home screen shortcut to the browser version on iOS](https://alexbarry.github.io/AlexCalc/add_to_ios_home.html).

## How to use

* **Complex numbers**: enter rectangular form (`3 + 4i`) or polar (`5 angle 30`). `j` can be used in place of `i` ([more info](https://en.wikipedia.org/wiki/Imaginary_unit)).
* **Variables**: store variables by typing an expression, then pressing the "store" button in the bottom left (or typing the `->` operator), and then entering a variable name (e.g. `1.23e5 -> x`). Then simply reference this variable name in other expressions (e.g. `x^2 - 4*x -> y`)
* **Units**: select or type a desired unit, e.g. `1 kg` or `3 m s^-2`. The default output is in SI units, but arbitrary unit conversion can be accomplished with the ` to ` operator (e.g. `1 kg to lb`) which can be typed directly or by pressing the **alt** then **to units** button (which appears in place of the **units** button when **alt** is pressed)
* **Negative numbers**: the "subtraction" button can also serve as a negative sign.

## Discussion

Please submit comments, feedback, feature requests, or discussions to:
* Github: https://github.com/alexbarry/AlexCalc
* Reddit: [reddit.com/r/AlexCalc](https://www.reddit.com/r/alexcalc/)
* Discord: [discord.gg/ckNstZc2nF](https://discord.gg/ckNstZc2nF)
* Email: `alexbarry.dev2 [ at ] gmail.com`
* Matrix: [`#alexcalc:matrix.org`](https://matrix.to/#/#alexcalc:matrix.org)

## How it works

* I wrote C++ to parse plaintext input (intended to be written with a keyboard or generated via button presses), like `3 + 4`, `sqrt(1 acre) to m` or `5*cos(45) + 3i -> x` into a set of nodes.
* I also wrote C++ to evaluate those nodes or convert them to plaintext LaTeX.
* I use [Emscripten](https://emscripten.org/) to compile the C++ to WebAssembly (WASM) for web, and [NDK](https://developer.android.com/ndk/guides) to compile to Android native code on Android.
* I use [MathJax](https://www.mathjax.org/) to render the LaTeX into the equation display in HTML. 
* I threw together a UI in HTML and Android. There turned out to be a lot more UI specific code than I anticipated. In hindsight I should have tried to do more of it in C++.

Once the page is downloaded, there are no server side dependencies. Everything should be running locally in your browser.

## Feedback, alternatives?

Please let me know if this is valuable to you (and what features are useful, and what are not useful or are buggy), or if a better alternative exists. This is something that I wanted for myself many years ago when I was in school, specifically to input polar complex numbers with degrees, either on my phone or PC.

The best I had at the time was an annoyingly expensive graphics calculator that I'd often forget to bring with me to class, or a PC running some programming language. Both of these had variables, neither had unit support. I remember it being particularly painful to enter polar complex numbers with the angle specified in degrees, I vaguely remember doing something like `e^(angle_in_degrees/1 radian)`.

The equation display also seems like a nice feature, since I'd often end up having to count seemingly endless brackets when entering even relatively simple formulas.

The units I threw in because it seemed like it should be easy, only to constantly get frustrated when dealing with things like degrees/radians and frequency in Hertz or radians/seconds, or wondering if multiplying by 1 degree Celsius should be the same as multiplying by 1 Kelvin (interpreting it as a change in temperature), or by 274 Kelvin (interpreting it as an absolute temperature).
