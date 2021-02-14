It works via homebridge.io using the `homebridge-http-switch` plugin.

Config is:
```
        {
            "accessory": "HTTP-SWITCH",
            "name": "Desk",
            "serialNumber": "42069",
            "switchType": "stateful",
            "onUrl": {
                "url": "http://desk.local/up/6",
                "method": "PUT"
            },
            "offUrl": {
                "url": "http://desk.local/down/6",
                "method": "PUT"
            },
            "statusUrl": {
                "url": "http://desk.local/status",
                "method": "GET"
            }
        }
```

6 is the amount of seconds the switch will be activated for. Adjust for your height/preference.
You will be able to say "Hey Siri, turn on/off my desk", create a set of Siri shortcuts to change that to a custom message.
