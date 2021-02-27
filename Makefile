.PHONY: release github-release

VERSION = $(shell LUA_PATH="./src/?.lua" lua -e 'print(require("luawav.version")._VERSION)')

github-release:
	source $(HOME)/.github-token && github-release release \
	  --user jprjr \
	  --repo luawav \
	  --tag v$(VERSION)
	source $(HOME)/.github-token && github-release upload \
	  --user jprjr \
	  --repo luawav \
	  --tag v$(VERSION) \
	  --name luawav-$(VERSION).tar.gz \
	  --file dist/luawav-$(VERSION).tar.gz
	source $(HOME)/.github-token && github-release upload \
	  --user jprjr \
	  --repo luawav \
	  --tag v$(VERSION) \
	  --name luawav-$(VERSION).tar.xz \
	  --file dist/luawav-$(VERSION).tar.xz

release:
	rm -rf dist/luawav-$(VERSION)
	rm -rf dist/luawav-$(VERSION).tar.gz
	rm -rf dist/luawav-$(VERSION).tar.xz
	mkdir -p dist/luawav-$(VERSION)/csrc
	rsync -a csrc/ dist/luawav-$(VERSION)/csrc/
	rsync -a src/ dist/luawav-$(VERSION)/src/
	rsync -a CMakeLists.txt dist/luawav-$(VERSION)/CMakeLists.txt
	rsync -a LICENSE dist/luawav-$(VERSION)/LICENSE
	rsync -a README.md dist/luawav-$(VERSION)/README.md
	sed 's/@VERSION@/$(VERSION)/g' < rockspec/luawav-template.rockspec > dist/luawav-$(VERSION)/luawav-$(VERSION)-1.rockspec
	cd dist && tar -c -f luawav-$(VERSION).tar luawav-$(VERSION)
	cd dist && gzip -k luawav-$(VERSION).tar
	cd dist && xz luawav-$(VERSION).tar

