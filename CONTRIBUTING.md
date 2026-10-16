# Contributing

Thanks for helping with Videoina.

By taking part you agree to the [Code of Conduct](CODE_OF_CONDUCT.md).

## Reporting a bug or asking for a feature

Open an [issue](https://github.com/Gitguy268/videoina/issues/new/choose) and use the template. For a bug, say which OS and Videoina version, what file type you played, what you did, what you expected and what happened.

Report security problems privately: see [SECURITY.md](SECURITY.md).

## Making a change

1. Fork the repository and create a branch from `main`.
2. Build it (see [Building](README.md#building) in the README).
3. Make your change. Keep commits small, each with a clear message.
4. Run the smoke test below and update the README if behaviour changed.
5. Open a pull request and fill in the template. CI builds Windows and Linux and uploads a screenshot from each; check them.

## Smoke test

```sh
ffmpeg -f lavfi -i testsrc2=duration=10:size=1280x720:rate=30 -f lavfi -i sine=duration=10 -shortest test.mkv
./build/videoina test.mkv --screenshot shot.png --screenshot-after 3000
```

It exits with 0 only if the video played and the screenshot was saved. Look at `shot.png`: the video should show through the glass control bar.

## Licence

Videoina is GPL-3.0. Contributions are accepted under the same licence. A new dependency must have a GPL-3.0-compatible licence and be listed in NOTICE.
