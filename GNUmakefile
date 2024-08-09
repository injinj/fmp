# fmp makefile
lsb_dist     := $(shell if [ -f /etc/os-release ] ; then \
                  grep '^NAME=' /etc/os-release | sed 's/.*=[\"]*//' | sed 's/[ \"].*//' ; \
                  elif [ -x /usr/bin/lsb_release ] ; then \
                  lsb_release -is ; else echo Linux ; fi)
lsb_dist_ver := $(shell if [ -f /etc/os-release ] ; then \
		  grep '^VERSION=' /etc/os-release | sed 's/.*=[\"]*//' | sed 's/[ \"].*//' ; \
                  elif [ -x /usr/bin/lsb_release ] ; then \
                  lsb_release -rs | sed 's/[.].*//' ; else uname -r | sed 's/[-].*//' ; fi)
#lsb_dist     := $(shell if [ -x /usr/bin/lsb_release ] ; then lsb_release -is ; else echo Linux ; fi)
#lsb_dist_ver := $(shell if [ -x /usr/bin/lsb_release ] ; then lsb_release -rs | sed 's/[.].*//' ; else uname -r | sed 's/[-].*//' ; fi)
uname_m      := $(shell uname -m)

short_dist_lc := $(patsubst CentOS,rh,$(patsubst RedHatEnterprise,rh,\
                   $(patsubst RedHat,rh,\
                     $(patsubst Fedora,fc,$(patsubst Ubuntu,ub,\
                       $(patsubst Debian,deb,$(patsubst SUSE,ss,$(lsb_dist))))))))
short_dist    := $(shell echo $(short_dist_lc) | tr a-z A-Z)
pwd           := $(shell pwd)
rpm_os        := $(short_dist_lc)$(lsb_dist_ver).$(uname_m)

# this is where the targets are compiled
build_dir ?= $(short_dist)$(lsb_dist_ver)_$(uname_m)$(port_extra)
bind      := $(build_dir)/bin
libd      := $(build_dir)/lib64
objd      := $(build_dir)/obj
dependd   := $(build_dir)/dep

have_asciidoctor := $(shell if [ -x /usr/bin/asciidoctor ] ; then echo true; fi)
have_rpm         := $(shell if [ -x /bin/rpmquery ] ; then echo true; fi)
have_dpkg        := $(shell if [ -x /bin/dpkg-buildflags ] ; then echo true; fi)
default_cflags   := -ggdb -O3
# use 'make port_extra=-g' for debug build
ifeq (-g,$(findstring -g,$(port_extra)))
  default_cflags := -ggdb
endif
ifeq (-a,$(findstring -a,$(port_extra)))
  default_cflags += -fsanitize=address
endif
ifeq (-mingw,$(findstring -mingw,$(port_extra)))
  CC    := /usr/bin/x86_64-w64-mingw32-gcc
  CXX   := /usr/bin/x86_64-w64-mingw32-g++
  mingw := true
endif
ifeq (,$(port_extra))
  ifeq (true,$(have_rpm))
    build_cflags = $(shell /bin/rpm --eval '%{optflags}')
  endif
  ifeq (true,$(have_dpkg))
    build_cflags = $(shell /bin/dpkg-buildflags --get CFLAGS)
  endif
endif
# msys2 using ucrt64
ifeq (MSYS2,$(lsb_dist))
  mingw := true
endif
CC          ?= gcc
CXX         ?= g++
cc          := $(CC) -std=c11
cpp         := $(CXX)
arch_cflags := -mavx -maes -fno-omit-frame-pointer
gcc_wflags  := -Wall -Wextra
#-Werror
# if windows cross compile
ifeq (true,$(mingw))
dll         := dll
exe         := .exe
soflag      := -shared -Wl,--subsystem,windows
fpicflags   := -fPIC -DMS_SHARED
sock_lib    := -lcares -lssl -lcrypto -lws2_32 -lwinmm -liphlpapi
dynlink_lib := -lpcre2-8 -lpcre2-32 -lz
NO_STL      := 1
else
dll         := so
exe         :=
soflag      := -shared
fpicflags   := -fPIC
thread_lib  := -pthread -lrt
sock_lib    := -lcares -lssl -lcrypto
dynlink_lib := -lpcre2-8 -lpcre2-32 -lz
endif
# make apple shared lib
ifeq (Darwin,$(lsb_dist)) 
dll         := dylib
endif
# rpmbuild uses RPM_OPT_FLAGS
#ifeq ($(RPM_OPT_FLAGS),)
CFLAGS ?= $(build_cflags) $(default_cflags)
#else
#CFLAGS ?= $(RPM_OPT_FLAGS)
#endif
cflags := $(gcc_wflags) $(CFLAGS) $(arch_cflags)
lflags := -Wno-stringop-overflow

INCLUDES  ?= -Iinclude
#-Iraikv/include -Iraimd/include -Iraids/include -Ilinecook/include
DEFINES   ?=
includes  := $(INCLUDES)
defines   := $(DEFINES)

# if not linking libstdc++
ifdef NO_STL
cppflags  := -std=c++11 -fno-rtti -fno-exceptions
cpplink   := $(CC)
else
cppflags  := -std=c++11
cpplink   := $(CXX)
endif

math_lib    := -lm

# test submodules exist (they don't exist for dist_rpm, dist_dpkg targets)
test_makefile = $(shell if [ -f ./$(1)/GNUmakefile ] ; then echo ./$(1) ; \
                        elif [ -f ../$(1)/GNUmakefile ] ; then echo ../$(1) ; fi)
md_home     := $(call test_makefile,raimd)
dec_home    := $(call test_makefile,libdecnumber)
kv_home     := $(call test_makefile,raikv)
ds_home     := $(call test_makefile,raids)
lc_home     := $(call test_makefile,linecook)
h3_home     := $(call test_makefile,h3)
rdb_home    := $(call test_makefile,rdbparser)
pgm_home    := $(call test_makefile,openpgm)
sassrv_home := $(call test_makefile,sassrv)
natsmd_home := $(call test_makefile,natsmd)
lzf_home    := $(call test_makefile,lzf)

ifeq (,$(dec_home))
dec_home    := $(call test_makefile,$(md_home)/libdecnumber)
endif
ifeq (,$(h3_home))
h3_home     := $(call test_makefile,$(ds_home)/h3)
endif
ifeq (,$(rdb_home))
rdb_home    := $(call test_makefile,$(ds_home)/rdbparser)
endif
ifeq (,$(lzf_home))
lzf_home    := $(call test_makefile,$(rdb_home)/lzf)
endif

lnk_lib     := -Wl,--push-state -Wl,-Bstatic
lnk_lib     += $(libd)/libfmp.a
dlnk_lib    :=
lnk_dep     := $(libd)/libfmp.a
dlnk_dep    :=

ifneq (,$(ds_home))
ds_lib      := $(ds_home)/$(libd)/libraids.a
ds_dll      := $(ds_home)/$(libd)/libraids.$(dll)
lnk_lib     += $(ds_lib)
lnk_dep     += $(ds_lib)
dlnk_lib    += -L$(ds_home)/$(libd) -lraids
dlnk_dep    += $(ds_dll)
rpath1       = ,-rpath,$(pwd)/$(ds_home)/$(libd)
ds_includes  = -I$(ds_home)/include
else
lnk_lib     += -lraids
dlnk_lib    += -lraids
endif

ifneq (,$(md_home))
md_lib      := $(md_home)/$(libd)/libraimd.a
md_dll      := $(md_home)/$(libd)/libraimd.$(dll)
lnk_lib     += $(md_lib)
lnk_dep     += $(md_lib)
dlnk_lib    += -L$(md_home)/$(libd) -lraimd
dlnk_dep    += $(md_dll)
rpath2       = ,-rpath,$(pwd)/$(md_home)/$(libd)
includes    += -I$(md_home)/include
else
lnk_lib     += -lraimd
dlnk_lib    += -lraimd
endif

ifneq (,$(dec_home))
dec_lib     := $(dec_home)/$(libd)/libdecnumber.a
dec_dll     := $(dec_home)/$(libd)/libdecnumber.$(dll)
lnk_lib     += $(dec_lib)
lnk_dep     += $(dec_lib)
dlnk_lib    += -L$(dec_home)/$(libd) -ldecnumber
dlnk_dep    += $(dec_dll)
rpath3       = ,-rpath,$(pwd)/$(dec_home)/$(libd)
dec_includes = -I$(dec_home)/include
else
lnk_lib     += -ldecnumber
dlnk_lib    += -ldecnumber
endif

ifneq (,$(lc_home))
lc_lib      := $(lc_home)/$(libd)/liblinecook.a
lc_dll      := $(lc_home)/$(libd)/liblinecook.$(dll)
lnk_lib     += $(lc_lib)
lnk_dep     += $(lc_lib)
dlnk_lib    += -L$(lc_home)/$(libd) -llinecook
dlnk_dep    += $(lc_dll)
rpath4       = ,-rpath,$(pwd)/$(lc_home)/$(libd)
lc_includes  = -I$(lc_home)/include
else
lnk_lib     += -llinecook
dlnk_lib    += -llinecook
endif

ifneq (,$(h3_home))
h3_lib      := $(h3_home)/$(libd)/libh3lib.a
h3_dll      := $(h3_home)/$(libd)/libh3lib.$(dll)
lnk_lib     += $(h3_lib)
lnk_dep     += $(h3_lib)
dlnk_lib    += -L$(h3_home)/$(libd) -lh3lib
dlnk_dep    += $(h3_dll)
rpath5       = ,-rpath,$(pwd)/$(h3_home)/$(libd)
h3_includes  = -I$(h3_home)/src/h3lib/include
else
lnk_lib     += -lh3lib
dlnk_lib    += -lh3lib
endif

ifneq (,$(rdb_home))
rdb_lib     := $(rdb_home)/$(libd)/librdbparser.a
rdb_dll     := $(rdb_home)/$(libd)/librdbparser.$(dll)
lnk_lib     += $(rdb_lib)
lnk_dep     += $(rdb_lib)
dlnk_lib    += -L$(rdb_home)/$(libd) -lrdbparser
dlnk_dep    += $(rdb_dll)
rpath6       = ,-rpath,$(pwd)/$(rdb_home)/$(libd)
rdb_includes = -I$(rdb_home)/include
else
lnk_lib     += -lrdbparser
dlnk_lib    += -lrdbparser
endif

ifneq (,$(kv_home))
kv_lib      := $(kv_home)/$(libd)/libraikv.a
kv_dll      := $(kv_home)/$(libd)/libraikv.$(dll)
lnk_lib     += $(kv_lib)
lnk_dep     += $(kv_lib)
dlnk_lib    += -L$(kv_home)/$(libd) -lraikv
dlnk_dep    += $(kv_dll)
rpath7       = ,-rpath,$(pwd)/$(kv_home)/$(libd)
includes    += -I$(kv_home)/include
else
lnk_lib     += -lraikv
dlnk_lib    += -lraikv
endif

ifneq (,$(pgm_home))
pgm_lib     := $(pgm_home)/$(libd)/libopenpgm_st.a
pgm_dll     := $(pgm_home)/$(libd)/libopenpgm_st.$(dll)
lnk_lib     += $(pgm_lib)
lnk_dep     += $(pgm_lib)
dlnk_lib    += -L$(pgm_home)/$(libd) -lopenpgm_st
dlnk_dep    += $(pgm_dll)
rpath8       = ,-rpath,$(pwd)/$(pgm_home)/$(libd)
pgm_includes = -I$(pgm_home)/openpgm/pgm/include
else
lnk_lib     += -lopenpgm_st
dlnk_lib    += -lopenpgm_st
endif

ifneq (,$(sassrv_home))
sassrv_lib  := $(sassrv_home)/$(libd)/libsassrv.a
sassrv_dll  := $(sassrv_home)/$(libd)/libsassrv.$(dll)
lnk_lib     += $(sassrv_lib)
lnk_dep     += $(sassrv_lib)
dlnk_lib    += -L$(sassrv_home)/$(libd) -lsassrv
dlnk_dep    += $(sassrv_dll)
rpath9       = ,-rpath,$(pwd)/$(sassrv_home)/$(libd)
sassrv_includes = -I$(sassrv_home)/include
else
lnk_lib     += -lsassrv
dlnk_lib    += -lsassrv
endif

ifneq (,$(natsmd_home))
natsmd_lib  := $(natsmd_home)/$(libd)/libnatsmd.a
natsmd_dll  := $(natsmd_home)/$(libd)/libnatsmd.$(dll)
lnk_lib     += $(natsmd_lib)
lnk_dep     += $(natsmd_lib)
dlnk_lib    += -L$(natsmd_home)/$(libd) -lnatsmd
dlnk_dep    += $(natsmd_dll)
rpath10      = ,-rpath,$(pwd)/$(natsmd_home)/$(libd)
natsmd_includes = -I$(natsmd_home)/include
else
lnk_lib     += -lnatsmd
dlnk_lib    += -lnatsmd
endif

lnk_lib += -Wl,--pop-state

ifneq (,$(lzf_home))
lzf_lib     := $(lzf_home)/$(libd)/liblzf.a
lzf_dll     := $(lzf_home)/$(libd)/liblzf.$(dll)
lnk_lib     += $(lzf_lib)
lnk_dep     += $(lzf_lib)
dlnk_lib    += -L$(lzf_home)/$(libd) -llzf
dlnk_dep    += $(lzf_dll)
rpath11      = ,-rpath,$(pwd)/$(lzf_home)/$(libd)
lzf_includes = -I$(lzf_home)/include
else
lnk_lib     += -llzf
dlnk_lib    += -llzf
includes    += -I/usr/include/liblzf
endif

rpath := -Wl,-rpath,$(pwd)/$(libd)$(rpath1)$(rpath2)$(rpath3)$(rpath4)$(rpath5)$(rpath6)$(rpath7)$(rpath8)$(rpath9)$(rpath10)$(rpath11)

.PHONY: everything
everything: $(kv_lib) $(dec_lib) $(lzf_lib) $(md_lib) $(lc_lib) $(h3_lib) $(rdb_lib) $(ds_lib) $(pgm_lib) $(sassrv_lib) $(natsmd_lib) all

clean_subs :=
dlnk_dll_depend :=
dlnk_lib_depend :=

# build submodules if have them
ifneq (,$(kv_home))
$(kv_lib) $(kv_dll):
	$(MAKE) -C $(kv_home)
.PHONY: clean_kv
clean_kv:
	$(MAKE) -C $(kv_home) clean
clean_subs += clean_kv
endif
ifneq (,$(dec_home))
$(dec_lib) $(dec_dll):
	$(MAKE) -C $(dec_home)
.PHONY: clean_dec
clean_dec:
	$(MAKE) -C $(dec_home) clean
clean_subs += clean_dec
endif
ifneq (,$(md_home))
$(md_lib) $(md_dll):
	$(MAKE) -C $(md_home)
.PHONY: clean_md
clean_md:
	$(MAKE) -C $(md_home) clean
clean_subs += clean_md
endif
ifneq (,$(ds_home))
$(ds_lib) $(ds_dll):
	$(MAKE) -C $(ds_home)
.PHONY: clean_ds
clean_ds:
	$(MAKE) -C $(ds_home) clean
clean_subs += clean_ds
endif
ifneq (,$(lc_home))
$(lc_lib) $(lc_dll):
	$(MAKE) -C $(lc_home)
.PHONY: clean_lc
clean_lc:
	$(MAKE) -C $(lc_home) clean
clean_subs += clean_lc
endif
ifneq (,$(h3_home))
$(h3_lib) $(h3_dll):
	$(MAKE) -C $(h3_home)
.PHONY: clean_h3
clean_h3:
	$(MAKE) -C $(h3_home) clean
clean_subs += clean_h3
endif
ifneq (,$(rdb_home))
$(rdb_lib) $(rdb_dll):
	$(MAKE) -C $(rdb_home)
.PHONY: clean_rdb
clean_rdb:
	$(MAKE) -C $(rdb_home) clean
clean_subs += clean_rdb
endif
ifneq (,$(pgm_home))
$(pgm_lib) $(pgm_dll):
	$(MAKE) -C $(pgm_home)
.PHONY: clean_pgm
clean_pgm:
	$(MAKE) -C $(pgm_home) clean
clean_subs += clean_pgm
endif
ifneq (,$(sassrv_home))
$(sassrv_lib) $(sassrv_dll):
	$(MAKE) -C $(sassrv_home)
.PHONY: clean_sassrv
clean_sassrv:
	$(MAKE) -C $(sassrv_home) clean
clean_subs += clean_sassrv
endif
ifneq (,$(natsmd_home))
$(natsmd_lib) $(natsmd_dll):
	$(MAKE) -C $(natsmd_home)
.PHONY: clean_natsmd
clean_natsmd:
	$(MAKE) -C $(natsmd_home) clean
clean_subs += clean_natsmd
endif
ifneq (,$(lzf_home))
$(lzf_lib) $(lzf_dll):
	$(MAKE) -C $(lzf_home)
.PHONY: clean_lzf
clean_lzf:
	$(MAKE) -C $(lzf_home) clean
clean_subs += clean_lzf
endif

# copr/fedora build (with version env vars)
# copr uses this to generate a source rpm with the srpm target
-include .copr/Makefile

# debian build (debuild)
# target for building installable deb: dist_dpkg
-include deb/Makefile

# targets filled in below
all_exes    :=
all_libs    :=
all_dlls    :=
all_depends :=
gen_files   :=

ws_includes := $(ds_includes)

libfmp_files   := ws
libfmp_files := $(libfmp_files)
libfmp_cfile := $(addprefix src/, $(addsuffix .cpp, $(libfmp_files)))
libfmp_objs  := $(addprefix $(objd)/, $(addsuffix .o, $(libfmp_files)))
libfmp_dbjs  := $(addprefix $(objd)/, $(addsuffix .fpic.o, $(libfmp_files)))
libfmp_deps  := $(addprefix $(dependd)/, $(addsuffix .d, $(libfmp_files))) \
                   $(addprefix $(dependd)/, $(addsuffix .fpic.d, $(libfmp_files)))
libfmp_dlnk  := $(dlnk_lib)
libfmp_spec  := $(version)-$(build_num)_$(git_hash)
libfmp_ver   := $(major_num).$(minor_num)

$(libd)/libfmp.a: $(libfmp_objs)
$(libd)/libfmp.$(dll): $(libfmp_dbjs) $(dlnk_dep)

all_libs    += $(libd)/libfmp.a $(libd)/libfmp.$(dll)
all_depends += $(libfmp_deps)

wstest_includes := $(ds_includes)
wstest_files := wstest
wstest_cfile := test/wstest.cpp
wstest_objs  := $(addprefix $(objd)/, $(addsuffix .o, $(wstest_files)))
wstest_deps  := $(addprefix $(dependd)/, $(addsuffix .d, $(wstest_files)))
wstest_libs  :=
wstest_lnk   := $(lnk_lib)

$(bind)/wstest$(exe): $(wstest_objs) $(wstest_libs) $(lnk_dep)

all_exes    += $(bind)/wstest$(exe)
all_depends += $(wstest_deps)

all_dirs := $(bind) $(libd) $(objd) $(dependd)

# the default targets
.PHONY: all
all: $(all_libs) $(all_dlls) $(all_exes)

.PHONY: dnf_depend
dnf_depend:
	sudo dnf -y install make gcc-c++ git redhat-lsb openssl-devel pcre2-devel chrpath c-ares-devel

.PHONY: yum_depend
yum_depend:
	sudo yum -y install make gcc-c++ git redhat-lsb openssl-devel pcre2-devel chrpath c-ares-devel

.PHONY: deb_depend
deb_depend:
	sudo apt-get install -y install make g++ gcc devscripts libpcre2-dev chrpath git lsb-release libssl-dev c-ares-dev

# create directories
$(dependd):
	@mkdir -p $(all_dirs)

# remove target bins, objs, depends
.PHONY: clean
clean: $(clean_subs)
	rm -r -f $(bind) $(libd) $(objd) $(dependd)
	if [ "$(build_dir)" != "." ] ; then rmdir $(build_dir) ; fi

.PHONY: clean_dist
clean_dist:
	rm -rf dpkgbuild rpmbuild

.PHONY: clean_all
clean_all: clean clean_dist

# force a remake of depend using 'make -B depend'
.PHONY: depend
depend: $(dependd)/depend.make

$(dependd)/depend.make: $(dependd) $(all_depends)
	@echo "# depend file" > $(dependd)/depend.make
	@cat $(all_depends) >> $(dependd)/depend.make

ifeq (SunOS,$(lsb_dist))
remove_rpath = rpath -r
else
ifeq (Darwin,$(lsb_dist))
remove_rpath = true
else
remove_rpath = chrpath -d
endif
endif

.PHONY: dist_bins
dist_bins: $(all_libs)
	$(remove_rpath) $(libd)/libfmp.$(dll)

.PHONY: dist_rpm
dist_rpm: srpm
	( cd rpmbuild && rpmbuild --define "-topdir `pwd`" -ba SPECS/fmp.spec )

# dependencies made by 'make depend'
-include $(dependd)/depend.make

ifeq ($(DESTDIR),)
# 'sudo make install' puts things in /usr/local/lib, /usr/local/include
install_prefix = /usr/local
else
# debuild uses DESTDIR to put things into debian/fmp/usr
install_prefix = $(DESTDIR)/usr
endif

install: dist_bins
	install -d $(install_prefix)/lib $(install_prefix)/bin
	install -d $(install_prefix)/include/fmp
	for f in $(libd)/libfmp.* ; do \
	if [ -h $$f ] ; then \
	cp -a $$f $(install_prefix)/lib ; \
	else \
	install $$f $(install_prefix)/lib ; \
	fi ; \
	done
	install -m 644 include/fmp/*.h $(install_prefix)/include/fmp

$(objd)/%.o: src/%.cpp
	$(cpp) $(cflags) $(cppflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.o: src/%.c
	$(cc) $(cflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.fpic.o: src/%.cpp
	$(cpp) $(cflags) $(fpicflags) $(cppflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.fpic.o: src/%.c
	$(cc) $(cflags) $(fpicflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.o: test/%.cpp
	$(cpp) $(cflags) $(cppflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(objd)/%.o: test/%.c
	$(cc) $(cflags) $(includes) $(defines) $($(notdir $*)_includes) $($(notdir $*)_defines) -c $< -o $@

$(libd)/%.a:
	ar rc $@ $($(*)_objs)

ifeq (Darwin,$(lsb_dist))
$(libd)/%.dylib:
	$(cpplink) -dynamiclib $(cflags) $(lflags) -o $@.$($(*)_dylib).dylib -current_version $($(*)_dylib) -compatibility_version $($(*)_ver) $($(*)_dbjs) $($(*)_dlnk) $(sock_lib) $(math_lib) $(thread_lib) $(malloc_lib) $(dynlink_lib) && \
	cd $(libd) && ln -f -s $(@F).$($(*)_dylib).dylib $(@F).$($(*)_ver).dylib && ln -f -s $(@F).$($(*)_ver).dylib $(@F)
else
$(libd)/%.$(dll):
	$(cpplink) $(soflag) $(rpath) $(cflags) $(lflags) -o $@.$($(*)_spec) -Wl,-soname=$(@F).$($(*)_ver) $($(*)_dbjs) $($(*)_dlnk) $(sock_lib) $(math_lib) $(thread_lib) $(malloc_lib) $(dynlink_lib) && \
	cd $(libd) && ln -f -s $(@F).$($(*)_spec) $(@F).$($(*)_ver) && ln -f -s $(@F).$($(*)_ver) $(@F)
endif

$(bind)/%$(exe):
	$(cpplink) $(cflags) $(lflags) $(rpath) -o $@ $($(*)_objs) -L$(libd) $($(*)_lnk) $(cpp_lnk) $(sock_lib) $(math_lib) $(thread_lib) $(malloc_lib) $(dynlink_lib)

$(dependd)/%.d: src/%.cpp
	$(cpp) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

$(dependd)/%.d: src/%.c
	$(cc) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

$(dependd)/%.fpic.d: src/%.cpp
	$(cpp) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).fpic.o -MF $@

$(dependd)/%.fpic.d: src/%.c
	$(cc) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).fpic.o -MF $@

$(dependd)/%.d: test/%.cpp
	$(cpp) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

$(dependd)/%.d: test/%.c
	$(cc) $(arch_cflags) $(defines) $(includes) $($(notdir $*)_includes) $($(notdir $*)_defines) -MM $< -MT $(objd)/$(*).o -MF $@

