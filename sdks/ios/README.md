# The test runner

The test runner is an objective-c app which embeds the runtime. It has a command line interface similar to the mono runtime, i.e.
<exe> <arguments>.

# The test harness

The test harness is a C# app which automates running a test suite. It
install the app on the simulator, runs it, and collects output into
a log file.

# The app builder

This is a C# app which is used to create an ios .app bundle.

# Make targets

Similar to the ones in xamarin-macios/tests

	* *action*-*what*-*where*-*project*

* Action

	* build-
	* install-
	* run-

* What

	* -ios-

* Where

	* -sim-
	* -dev-

* Project

	* corlib etc.

The test apps require the corresponding test assembly to be already
built, i.e. by running make PROFILE=monotouch test in mcs/class/corlib
etc.

# Running tests on device

* The test apps need to be signed using a real signing identity instead
of the default ad-hoc signing. It also needs to include a provisioning
profile. This can be done using:

make build-ios-dev-<app> IOS_SIGNING_IDENTITY="iPhone Developer: XXX" IOS_PROVISIONING_PROFILE="$HOME/Library/MobileDevice/Provisioning Profiles/test.mobileprovision"

* The certificates/provisioning profiles need to be installed on the
host/device.

* The app is installed/run using 'ios-deploy'.

* The host and the device needs to be on the same network. This is
needed because the test results are sent back over a tcp connection.

Other possible approaches for returning results:

* Using the ios os_log facility. Unfortunately, the command line 'log' tool cannot
seem to read the device logs, only the graphical Console app can.
* Using `idevicesyslog` from `libimobiledevice`.
* Have the app listen on a port, and have the test harness connect to it using
`libimobiledevice`, i.e. https://github.com/rsms/peertalk.
* Use a publish-subscribe pattern by uploading test results to some cloud service like
Azure EventHub. This only requires client side internet access on the device and
the test harness.

## Notes

Bernhard needed this patch to make it work on his machine: https://gist.github.com/lewurm/7711187deadb57bae97a4d4be4df563b


## Notes for watchOS deploy via mlaunch

* receive system log (profile must be installed from https://developer.apple.com/bug-reporting/profiles-and-logs/?name=sysdiagnose ):
```console
$ mlaunch --devname=watch4 --logdev
```

* install bundle
```console
$ mlaunch -v -v --device watchos --installdev /Users/lewurm/work/mono-watch4-ios-sdk/sdks/ios/bin/watch-dev/test-Mono.Runtime.Tests.app
$ mlaunch -v -v -v -v -v -v -v -sdkroot /Applications/Xcode101.app/Contents/Developer --device watchos --installdev ./bin/ios-dev/test-Mono.Runtime.Tests.app/Watch/test-watch-Mono.Runtime.Tests.app
```

* launch bundle
```console
$ mlaunch -v -v --killdev com.xamarin.mono.ios.test-Mono.Runtime.Tests.watchkitapp \
    --launchdev ./bin/ios-dev/test-Mono.Runtime.Tests.app/Watch/test-watch-Mono.Runtime.Tests.app \
	--devname=watch4 --argument
$ mlaunch -v -v -v -v -v -v -v -sdkroot /Applications/Xcode101.app/Contents/Developer --killdev com.xamarin.mono.ios.test-Mono.Runtime.Tests.watchkitapp --launchdev ./bin/ios-dev/test-Mono.Runtime.Tests.app/Watch/test-watch-Mono.Runtime.Tests.app --devname=watch4 --argument=-exclude:MobileNotWorking,NotOnMac,NotWorking,ValueAdd,CAS,InetAccess,NotWorkingLinqInterpreter --argument=-labels --argument=monotouch_Mono.Runtime.Tests_test.dll
```


Tip: Avoid trailing slash on bundle directory
