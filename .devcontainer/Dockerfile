FROM mcr.microsoft.com/devcontainers/base:trixie

# Install LLVM and clangd.
# https://apt.llvm.org/
RUN bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"

RUN apt-get update && \
    apt-get install -y \
    # Popular C library which provides commonly used data structures (dynamic string, dynamic array, hashmap, ...)
    libglib2.0-dev

# Use `zsh` as the default shell.
RUN chsh -s $(which zsh) vscode

# `vscode` is a built-in user in the base image.
USER vscode

# Homebrew installer must be run as non-root user, as instructed by the maintainer.
# https://docs.brew.sh/FAQ#why-does-homebrew-say-sudo-is-bad
RUN NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" && \
    # Add `brew` command to the PATH of zsh (default shell).
    echo 'eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv zsh)"' >> /home/vscode/.zshrc && \
    # Enable access to `brew` command and install packages.
    eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)" && \
    brew install gh go-task && \
    # Remove all cache files to reduce image size.
    brew cleanup --prune=all

# Install Nand2Tetris software suite (JackCompiler.sh, VMEmulator.sh, etc.).
RUN curl -fsSL "https://drive.usercontent.google.com/download?id=1xZzcMIUETv3u3sdpM_oTJSTetpVee3KZ&confirm=t" -o /tmp/nand2tetris.zip && \
    unzip -q /tmp/nand2tetris.zip -d /home/vscode && \
    rm /tmp/nand2tetris.zip && \
    chmod +x /home/vscode/nand2tetris/tools/*.sh && \
    echo 'export PATH="$HOME/nand2tetris/tools:$PATH"' >> /home/vscode/.zshrc
