# ~/.zshrc - RayuOS Zsh Configuration for GNOME Terminal and Shells

# Set up the prompt
setopt PROMPT_SUBST
PS1='%n@%m:%~%# '

# History settings
HISTSIZE=1000
SAVEHIST=2000
setopt HIST_IGNORE_DUPS

# Source global profile
if [ -f /etc/profile ]; then
    . /etc/profile
fi

# Source profile.d scripts
if [ -d /etc/profile.d ]; then
    for rc in /etc/profile.d/*.sh; do
        if [ -f "$rc" ]; then
            . "$rc"
        fi
    done
fi

# Color support
if [ -x /usr/bin/dircolors ]; then
    eval "$(dircolors -b)"
fi

# Useful aliases
alias ls='ls --color=auto'
alias grep='grep --color=auto'
alias ll='ls -alF'
alias la='ls -A'

# Source zshrc.d if exists
if [ -d ~/.zshrc.d ]; then
    for rc in ~/.zshrc.d/*.sh; do
        if [ -f "$rc" ]; then
            . "$rc"
        fi
    done
fi
