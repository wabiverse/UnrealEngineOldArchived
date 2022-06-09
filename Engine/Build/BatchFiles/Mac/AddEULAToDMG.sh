#/bin/sh

DMG_PATH=$1
DMG_TEMP_PATH="${DMG_PATH}.temp.dmg"
EULA_PATH=$2

# Unflatten the image
hdiutil unflatten "$DMG_PATH"

# Append the rez file containing the EULA
xcrun Rez -isysroot /Applications/Xcode-beta.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX13.0.sdk Carbon.r "$EULA_PATH" -a -o "$DMG_PATH"

# Flatten the image again
hdiutil flatten "$DMG_PATH"

exit $?
