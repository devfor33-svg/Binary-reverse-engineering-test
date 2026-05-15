CXX ?= g++
CXXFLAGS ?= -Wall -O2 -std=c++17

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Prefer system apt-pkg headers, fallback to Termux
ifneq ($(wildcard /usr/include/apt-pkg/cmndline.h),)
    APT_PKG_CFLAGS :=
    APT_PKG_LDFLAGS := -lapt-pkg
else ifneq ($(wildcard /data/data/com.termux/files/usr/include/apt-pkg/cmndline.h),)
    TERMUX_PREFIX := /data/data/com.termux/files/usr
    APT_PKG_CFLAGS := -I$(TERMUX_PREFIX)/include
    APT_PKG_LDFLAGS := -L$(TERMUX_PREFIX)/lib -lapt-pkg -Wl,-rpath=$(TERMUX_PREFIX)/lib
endif

CXXFLAGS += $(APT_PKG_CFLAGS)
LDFLAGS += $(APT_PKG_LDFLAGS)

TARGET = apt

.PHONY: all clean test install termux-build static

all: $(TARGET)

$(TARGET): main.cc
	$(CXX) $(CXXFLAGS) -o $@ main.cc $(LDFLAGS)

clean:
	rm -f $(TARGET) main.o

test: $(TARGET)
	@echo "=== Test 1: --help ==="
	./$(TARGET) --help
	@echo ""
	@echo "=== Test 2: moo ==="
	./$(TARGET) moo
	@echo ""
	@echo "=== Test 3: list ==="
	./$(TARGET) list 2>&1 | head -20
	@echo ""
	@echo "=== Test 4: search (busybox) ==="
	./$(TARGET) search busybox 2>&1 | head -10
	@echo ""
	@echo "=== Test 5: show bash ==="
	./$(TARGET) show bash 2>&1 | head -10
	@echo ""
	@echo "=== Test 6: depends ==="
	./$(TARGET) depends bash 2>&1 | head -10
	@echo ""
	@echo "=== Test 7: rdepends ==="
	./$(TARGET) rdepends bash 2>&1 | head -10
	@echo ""
	@echo "=== Test 8: policy ==="
	./$(TARGET) policy bash 2>&1 | head -10
	@echo ""
	@echo "=== All tests passed ==="

# Static build for portability
static: CXXFLAGS += -static-libstdc++ -static-libgcc
static: LDFLAGS += -static
static: $(TARGET)

# Termux build
termux-build:
	$(MAKE) CXX=aarch64-linux-android21-clang++ \
		CXXFLAGS="-Wall -O2 -std=c++17 -I$(TERMUX_PREFIX)/include" \
		LDFLAGS="-L$(TERMUX_PREFIX)/lib -lapt-pkg -static-libstdc++"
