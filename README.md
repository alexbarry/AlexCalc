# AlexCalc

Try the web version here: https://alexbarry.github.io/AlexCalc/

Also available for Android on [Google Play](https://play.google.com/store/apps/details?id=net.alexbarry.calc_android)

For iOS [how to add a shortcut to the browser version on iOS here](https://alexbarry.github.io/AlexCalc/add_to_ios_home.html).

## How it works

* I wrote C++ to parse plaintext input (intended to be written with a keyboard), like `3 + 4` or `sqrt(1 acre) to miles` into a set of nodes.
* I also wrote C++ to evaluate those nodes or convert it to plaintext LaTeX.
* I use [Emscripten](https://emscripten.org/) to compile the C++ to WebAssembly (WASM).
* I use [MathJax](https://www.mathjax.org/) to render the LaTeX into the equation display.

Once the page is loaded, there are no server side dependencies. Everything (should be) running locally in your browser.
