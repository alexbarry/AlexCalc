# AlexCalc

Try the web version here: https://alexbarry.github.io/AlexCalc/

Also available for Android on [Google Play](https://play.google.com/store/apps/details?id=net.alexbarry.calc_android)

For iOS [how to add a shortcut to the browser version on iOS here](https://alexbarry.github.io/AlexCalc/add_to_ios_home.html).

# Discussion

Please submit comments, feedback, feature requests, or discussions to:
* Github: https://github.com/alexbarry/AlexCalc
* Reddit: [reddit.com/r/AlexCalc](https://www.reddit.com/r/alexcalc/)
* Discord: [discord.gg/ckNstZc2nF](https://discord.gg/ckNstZc2nF)
* Matrix: [`#alexcalc:matrix.org`](https://matrix.to/#/#alexcalc:matrix.org)

## How it works

* I wrote C++ to parse plaintext input (intended to be written with a keyboard), like `3 + 4` or `sqrt(1 acre) to miles` into a set of nodes.
* I also wrote C++ to evaluate those nodes or convert them to plaintext LaTeX.
* I use [Emscripten](https://emscripten.org/) to compile the C++ to WebAssembly (WASM).
* I use [MathJax](https://www.mathjax.org/) to render the LaTeX into the equation display.
* I threw together a UI in HTML and Android. There turned out to be a lot more UI specific code than I anticipated. In hindsight I should have tried to do more of it in C++.

Once the page is downloaded, there are no server side dependencies. Everything should be running locally in your browser.
