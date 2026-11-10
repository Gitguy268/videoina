# Flatpak

A self-hosted Flatpak of Videoina. It isn't on Flathub. Tagged releases carry an
`io.github.gitguy268.videoina.flatpak` file, built by `.github/workflows/build.yml`.

```sh
flatpak remote-add --user --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user ./io.github.gitguy268.videoina.flatpak
flatpak run io.github.gitguy268.videoina
```

It can read your Videos, Music and Downloads folders; for files elsewhere, use the Open dialog
(`flatpak override --user --filesystem=home io.github.gitguy268.videoina` opens up your home folder).

Build it yourself on Linux, from the repo root:

```sh
flatpak-builder --user --install-deps-from=flathub --force-clean --repo=repo build packaging/flatpak/io.github.gitguy268.videoina.yml
flatpak build-bundle --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo repo videoina.flatpak io.github.gitguy268.videoina
```
