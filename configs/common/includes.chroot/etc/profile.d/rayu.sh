# RayuOS shell function: user-friendly wrapper around the rayu CLI
# - Implements `rayu goto <DIR>` that changes the current shell directory
# - Adds friendly forms like `rayu show file [DIR]`
# - For `rayu apt ...`, elevates via sudo when available

rayu() {
  # Ensure we're a function, not the binary
  # Handle 'goto' (must be in shell to affect cwd)
  if [ "$1" = "goto" ]; then
    shift
    if [ -z "$1" ]; then
      echo "Usage: rayu goto <DIR>" >&2
      return 1
    fi
    builtin cd -- "$1" || return $?
    return 0
  fi

  # Friendly 'show file' alias
  if [ "$1" = "show" ] && { [ "$2" = "file" ] || [ "$2" = "files" ]; }; then
    local target="."
    [ -n "$3" ] && target="$3"
    ls -la -- "$target"
    return $?
  fi

  # Pass-through to apt with sudo if available
  if [ "$1" = "apt" ]; then
    shift
    if command -v sudo >/dev/null 2>&1; then
      sudo apt "$@"
    else
      apt "$@"
    fi
    return $?
  fi

  # Default: call the rayu binary for other commands
  if [ -x /usr/local/bin/rayu ]; then
    /usr/local/bin/rayu "$@"
  else
    command rayu "$@"  # fallback if installed elsewhere in PATH
  fi
}

export -f rayu 2>/dev/null || true
