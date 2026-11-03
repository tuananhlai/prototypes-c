#!/usr/bin/zsh

set -e

# Set the ZSH theme to "robbyrussell".
sed -i '/^ZSH_THEME/c\ZSH_THEME="robbyrussell"' ~/.zshrc
# Disable Address Space Layout Randomization due to permission error when running inside a container.
echo "settings set target.disable-aslr false" >> ~/.lldbinit