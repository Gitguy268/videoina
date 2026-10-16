# Security policy

## Reporting a vulnerability

Please report security problems privately, not in a public issue.

Use GitHub's private reporting form: <https://github.com/Gitguy268/videoina/security/advisories/new>

Include what you found, how to reproduce it, the Videoina version and your OS. You should get a reply within a week. Please give the fix time to ship before you publish details.

## Scope

The most useful reports are about:

- a media file, playlist or URL that makes Videoina run code or read or write files it should not;
- the release downloads (the Windows zip and the Linux AppImage) or the GitHub Actions workflows that build them.

Bugs in decoding itself usually belong to [mpv](https://github.com/mpv-player/mpv/security) or [FFmpeg](https://ffmpeg.org/security.html); report them there too, and tell us so we can update the bundled copy.

## Supported versions

Only the latest release is supported.
