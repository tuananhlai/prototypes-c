#!/usr/bin/zsh

set -e

# Disable Address Space Layout Randomization due to permission error when running inside a container.
echo "settings set target.disable-aslr false" >> ~/.lldbinit

# Configure ZSH theme & plugins.
source ~/.oh-my-zsh/oh-my-zsh.sh
omz theme set fino
omz plugin enable timer