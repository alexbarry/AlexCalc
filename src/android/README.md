# How to download javascript dependencies

Run:

    cd app/src/main/assets/html/
    npm install

# Generate and Copy "other licenses"

    src/tools/generate_android_license_file.py
    cp combined_other_licenses.html src/android/app/src/main/assets/html/

# Other files

Copy this, but **remove the "back to calculator" link**, when clicking it I saw a strict mode error.

    cp src/html/help.html src/android/app/src/main/assets/html/

# Signed build

To build a signed build, ensure that `keystore.properties` is in the android project root and contains these lines:

	keyAlias=
	keyPassword=
	storeFile=keys.lnk
	storePassword=

With the values added after the equal signs, and no quotes or anything added.
Also ensure that `$ANDROID_ROOT/app/keys.lnk` is a symlink pointing to the exported keystore.

    ln -s <path to your keystore> app/keys.lnk

I couldn't figure out how to use an absolute file path in `keystore.properties`.

# Known issues

* In the really old emulator (2.7 QVGA API 23), if you select multiple lines of text and click on it, the application crashes with "InvalidArgumentException" and a -1 in some cursor position. I don't think this was an issue in my application.
