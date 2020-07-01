# Build Redis as follows:
#
# - make               -- create non-SGX no-debug-log manifest
# - make SGX=1         -- create SGX no-debug-log manifest
# - make SGX=1 DEBUG=1 -- create SGX debug-log manifest
#
# Any of these invocations clones Redis' git repository and builds Redis in
# default configuration and in the latest-to-date (5.0.5) version.
#
# By default, Redis uses poll/epoll mechanism of Linux. To build Redis with
# select, use `make USE_SELECT=1`. For correct re-builds, always clean up
# Redis source code beforehand via `make distclean`.
#
# Use `make clean` to remove Graphene-generated files and `make distclean` to
# additionally remove the cloned Redis git repository.

################################# CONSTANTS ###################################

# Relative path to Graphene root
GRAPHENEDIR = ../..
SGX_SIGNER_KEY ?= $(GRAPHENEDIR)/Pal/src/host/Linux-SGX/signer/enclave-key.pem

SRCDIR = src
COMMIT = 5.0.5
TAR_SHA256 = 3313d5e09938dc6e16c6911efa4a24b1c494bacb5a917a8e7925ebd55e56be85

ifeq ($(DEBUG),1)
GRAPHENEDEBUG = inline
else
GRAPHENEDEBUG = none
endif

.PHONY: all
all: secure_main secure_main.manifest pal_loader
ifeq ($(SGX),1)
all: secure_main.manifest.sgx
endif

include ../../Scripts/Makefile.configs

################################ REDIS MANIFEST ###############################

# The template file contains almost all necessary information to run Redis
# under Graphene / Graphene-SGX. We create redis.manifest (to be run under
# non-SGX Graphene) by simply replacing variables in the template file via sed.

secure_main.manifest: secure_main.manifest.template
	sed -e 's|$$(GRAPHENEDIR)|'"$(GRAPHENEDIR)"'|g' \
		-e 's|$$(GRAPHENEDEBUG)|'"$(GRAPHENEDEBUG)"'|g' \
		-e 's|$$(ARCH_LIBDIR)|'"$(ARCH_LIBDIR)"'|g' \
		$< > $@

# Manifest for Graphene-SGX requires special "pal-sgx-sign" procedure. This
# procedure measures all Redis dependencies (shared libraries and trusted
# files), measures Redis code/data pages, and adds measurements in the
# resulting manifest.sgx file (among other, less important SGX options).
#
# Additionally, Graphene-SGX requires EINITTOKEN and SIGSTRUCT objects (see
# SGX hardware ABI, in particular EINIT instruction). The "pal-sgx-get-token"
# script generates these objects and puts them in files .token and .sig
# respectively. Note that filenames must be the same as the executable/manifest
# name (i.e., "secure_main").

secure_main.manifest.sgx: secure_main.manifest
	$(GRAPHENEDIR)/Pal/src/host/Linux-SGX/signer/pal-sgx-sign \
		-libpal $(GRAPHENEDIR)/Runtime/libpal-Linux-SGX.so \
		-key $(SGX_SIGNER_KEY) \
		-manifest $< -output $@ \
		-exec secure_main
	$(GRAPHENEDIR)/Pal/src/host/Linux-SGX/signer/pal-sgx-get-token \
		-output secure_main.token -sig secure_main.sig

########################### COPIES OF EXECUTABLES #############################

# Redis build process creates the final executable as src/secure_main. For
# simplicity, copy it into our root directory.
# Also, create a link to pal_loader for simplicity.

secure_main:
	g++ -o secure_main secure_main.cpp -I/home/sam/Documents/duckdb/src/include -L/home/sam/Documents/duckdb/build/release/src/ -lduckdb

pal_loader:
	ln -s $(GRAPHENEDIR)/Runtime/pal_loader $@

############################## RUNNING TESTS ##################################
.PHONY: start-native-server
start-native-server: all
	./secure_main --save '' --protected-mode no

.PHONY: start-graphene-server
start-graphene-server: all
	./pal_loader secure_main --save '' --protected-mode no

################################## CLEANUP ####################################

.PHONY: clean
clean:
	$(RM) *.token *.sig *.manifest.sgx *.manifest pal_loader secure_main *.rdb

.PHONY: distclean
distclean: clean
	$(RM) -r $(SRCDIR) redis.tar.gz