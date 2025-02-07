# xSkins: External Knife & Skin Changer for CSGO

## Features

- **Optimized Performance**: Faster and more efficient than other external skin changers.
- **Self-Updating**: Utilizes the external memory library [xLiteMem](https://github.com/0xf1a/xLiteMem) for automatic updates.
- **Console Menu**: A compact, easy-to-use console menu for selecting knife models and skins.
- **Skin Loader**: Load skin values directly from a `.txt` file with the integrated parser.

## How It Works

The `m_hMyWeapons` array stores handles to all weapons equipped by the local player. By applying skin and model values to these weapon entities, we can change skins and models without worrying about which weapon the player is currently holding. This method ensures smooth application of skins without needing to modify the `fullupdate` offset. Finally, the viewmodel entity of the current weapon is set to the model index of the selected knife model.

## Fix for Visual Studio 2017+ Compatibility

Visual Studio 2017 (version 15.5 and later) introduced the `/permissive-` flag, which disallows implicit conversions from string literals to non-const `char*`. To resolve this, you can disable this flag in the project properties while you update your code to conform to the C++ standard. This setting is found under "Conformance mode" in the "Language" tab under "C/C++" in your project's properties.

## Credits

- **BuckshotYT** for the [GetModelIndex method](https://www.unknowncheats.me/forum/counterstrike-global-offensive/212036-model-indices-properly-externally.html)
- **tracersgta** for the [formatted skin names](https://www.unknowncheats.me/forum/counterstrike-global-offensive/300854-skin-list-ids-comments.html)
- **adamb70** for the [up-to-date skin dumps](https://github.com/adamb70/CSGO-skin-ID-dumper/)

## Important Disclaimer

Use this software at your own risk. I am not responsible for any potential bans or account issues that may arise from using this tool.
