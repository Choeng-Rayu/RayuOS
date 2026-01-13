# ~/.bashrc - RayuOS Bash Configuration for GNOME Terminal and Shells

# If not running interactively, don't do anything
case $- in
    *i*) ;;
      *) return;;
esac

# History settings
HISTCONTROL=ignoredups:ignorespace
HISTSIZE=1000
HISTFILESIZE=2000
shopt -s histappend
shopt -s checkwinsize

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

# Enable color support
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    alias ls='ls --color=auto'
    alias grep='grep --color=auto'
fi

# Useful aliases
alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'

# PS1 Prompt
PS1='\u@\h:\w\$ '

# Source bashrc.d if exists
if [ -d ~/.bashrc.d ]; then
    for rc in ~/.bashrc.d/*.sh; do
        if [ -f "$rc" ]; then
            . "$rc"
        fi
    done
fi
