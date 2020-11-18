SUBDIRS = src/modules

all clean:
	list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		$(MAKE) -s -C $$subdir depend || exit 1; \
		$(MAKE) -C $$subdir $@ || exit 1; \
	done

distclean:
	rm -f mlt-config packages.dat
	list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		$(MAKE) -C $$subdir $@ || exit 1; \
	done
	echo > config.mak

dist-clean: distclean

include config.mak

install:
	install -d "$(DESTDIR)$(libdir)"
	install -d "$(DESTDIR)$(moduledir)"
ifeq ($(extra_versioning), true)
	ln -s "$(moduledir)" "$(DESTDIR)$(unversionedmoduledir)"
endif
	install -d "$(DESTDIR)$(libdir)/pkgconfig"
	install -d "$(DESTDIR)$(mltdatadir)"
ifeq ($(extra_versioning), true)
	ln -s "$(mltdatadir)" "$(DESTDIR)$(unversionedmltdatadir)"
endif
	install -c -m 644 *.pc "$(DESTDIR)$(libdir)/pkgconfig"
	list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		$(MAKE) DESTDIR=$(DESTDIR) -C $$subdir $@ || exit 1; \
	done
ifneq ("x$(kdenliveeffects)", "x")
	install -c -m 644 data/typewriter.xml "$(DESTDIR)$(kdenliveeffects)"
endif

uninstall:
	rm -f "$(DESTDIR)$(libdir)"/pkgconfig/mlt-extra_modules.pc
	list='$(SUBDIRS)'; \
	for subdir in $$list; do \
		$(MAKE) DESTDIR=$(DESTDIR) -C $$subdir $@ || exit 1; \
	done
	rm -f "$(DESTDIR)$(mltdatadir)/typewriter.yml"
ifeq ($(compat_dirs), true)
	rm -rf "$(DESTDIR)$(prefix)/share/mlt"
endif
ifneq ("x$(kdenliveeffects)", "x")
	rm -f "$(DESTDIR)$(kdenliveeffects)/typewriter.xml"
endif

dist:
	git archive --format=tar --prefix=mlt-$(version)/ v$(version) | gzip >mlt-$(version).tar.gz

validate-yml:
	for file in $$(find src/modules -type f -name \*.yml \! -name resolution_scale.yml); do \
		echo "validate: $$file"; \
		kwalify -f src/framework/metaschema.yaml $$file || exit 1; \
	done

codespell:
	codespell -w -q 3 \
    -L shotcut,sav,boundry,percentil,readded,uint,ith,sinc,amin,childs,seeked,writen \
    -S ChangeLog,cJSON.c,cJSON.h,RtAudio.cpp,RtAudio.h,*.rej,mlt_wrap.*
