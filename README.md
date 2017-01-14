# `slax`
The Slack Events API Explorer

`slax` is a CLI tool to help you build, understand, and debug Slack applications that use the Slack Events API. This tool can help you answer questions such as:

* What fields does this event have?
* Why isn't my app properly responding to this event?
* Was the event even actually fired?

`slax` is designed to stand in front of your Slack app. It stands up an HTTP server that Slack can send events directly to, and can optionally forward those events to your code running elsewhere. It is designed to be the first thing you stand up (it knows how to respond to Slack's challenge events), and will help you build your Events API-driven app that much more quickly.

## Installing

What? You don't want to build from source? You want an easy method to get your grubby hands on a pre-built binary? I am very sorry to say that at this moment, you are out of luck. You must build from source. For now. See the section on building from source below.

## Using

There is one require parameter, and several optional parameters to `slax`. You will need to know the *verification token* that Slack has assigned to your app: You can find it on the front of the [app configuration page](http://TODO).

Once you have that, it is easy! If your verification token is `abc-123`, then:

```bash
$ slax abc-123
```

### Port and Endpoint

The deafult port is `8023`, and the default endpoint is `/`. These are, of course configurable:

```bash
$ slax abc-123 -p 8990 -e /events
```

will set up a server at `http://localhost:8990/events`

### Forwarding

Finally, if you have some code running somewhere, be it on your local machine, or out there in the ~~butt~~cloud somewhere, you can interpose `slax` between Slack and your actual running app—that is, you can configure `slax` to forward the content it receives from Slack onwards to your app.

For example, if you have an app running at `https://myawesomeapp.heroku.com/events` you can do this:

```bash
$ slax abc-123 -f https://myawesomeapp.heroku.com/events
```

*Note*: not all of Slack's headers are forwarded. Indeed, at the moment, none of them are. This is a known issue that will be addressed soon.

### Configuring Slack

Now, of course, before Slack will send events to `slax` you will need to configure the webhook endpoint for your app to point at `slax`. And, there are two wrinkles here: First of all, of course Slack isn't going to have much luck connecting to `http://localhost`; moreover, the endpoint must be secured with TLS.

Your best bet, therefore, is `ngrok`. This is another tool that will create a public (and secure) endpoint for a web service (like `slax`) running somewhere otherwise inaccessible. It punches holes through things (in a reasonable and secure way!) so that Slack can interact with code on your laptop.

If you don't know what that is, you can read more about this amazing tool [here](http://TODO). I'll wait here while you go read about it and install it.

So, if `slax` is running on port 8023, you can configure `ngrok` as:

```bash
$ ngrok http 8023
```

Just note the URL that `ngrok` gives you, and feed that to Slack!

## Building

Oh, so you want to build it yourself, from source! Good on you. `slax` requires a modern C++ compiler, `cmake` and `conan` to build.

*Note* that at the moment, some of `slax`'s dependencies not compile on Windows. I think. I would love some help on that front!

```
$ conan install --build=missing
$ cmake .
$ cmake --build .
$ ./bin/slax
```
