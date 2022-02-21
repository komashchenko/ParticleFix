# ParticleFix
A Valve Server Plugin to fix problems with particles, it consists of three submodules
+ LimitExtender - increase `ParticleEffectNames` table limit
+ PrecacheFix - fixes map particles precache and uncache unused particles
+ ClientPrecacheFix - fixes not working particles when the player loads them for the first time

## Installation
1.	Grab the [latest release package](https://github.com/komashchenko/ParticleFix/releases)
2.	Extract the package and upload it to your gameserver
3.	Use `plugin_print` to check if the plugin works

## Build instructions
Just as any other [AMBuild](https://wiki.alliedmods.net/AMBuild) project:
1. [Install AMBuild](https://wiki.alliedmods.net/AMBuild#Installation)
2. Download [CS:GO SDK](https://github.com/Wend4r/hl2sdk)
3. `mkdir build && cd build`
4. `python ../configure.py --enable-optimize --hl2sdk-root=???`
5. `ambuild`
