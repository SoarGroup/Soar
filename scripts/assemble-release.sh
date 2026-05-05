#!/usr/bin/env bash
# Assemble a Soar multiplatform release zip from per-platform cmake install
# trees and cloned external repos.
#
# Usage: ./scripts/assemble-release.sh <version> <platforms-dir> [repos-dir]
#
# <platforms-dir> must contain subdirectories named:
#   soar-win_x86-64  soar-linux_x86-64  soar-mac_x86-64  soar-mac_ARM64
# Each is a cmake --install prefix tree (bin/ lib/ include/ java/ tcl/ ...).
#
# [repos-dir] (optional) contains clones of SoarGroup repos:
#   Agents  Release-Support  VisualSoar  Domains-Eaters-TankSoar

set -euo pipefail

VERSION="${1:?Usage: $0 <version> <platforms-dir> [repos-dir]}"
PLATFORMS_DIR="${2:?Usage: $0 <version> <platforms-dir> [repos-dir]}"
REPOS_DIR="${3:-}"

SUITE="SoarSuite_${VERSION}-Multiplatform"
rm -rf "$SUITE"
mkdir -p "$SUITE"

PLATFORM_IDS=("win_x86-64" "linux_x86-64" "mac_x86-64" "mac_ARM64")

# ── Per-platform binaries → bin/<platform>/ ──────────────────────────────────

for PLAT in "${PLATFORM_IDS[@]}"; do
  SRC="$PLATFORMS_DIR/soar-${PLAT}"
  DEST="$SUITE/bin/$PLAT"
  mkdir -p "$DEST"

  if [ ! -d "$SRC" ]; then
    echo "WARNING: Platform $PLAT not found at $SRC, skipping"
    continue
  fi

  echo "=== Assembling $PLAT ==="

  # CLI executable + tests
  find "$SRC/bin" -maxdepth 1 -type f \
    \( -name "soar" -o -name "soar.exe" \
       -o -name "UnitTests" -o -name "UnitTests.exe" \) \
    -exec cp -v {} "$DEST/" \;

  # UnitTests test-agent fixtures (looked up via "./SoarUnitTests/" relative
  # to UnitTests.exe's CWD — see UnitTests/SoarHelpers/SoarHelper.cpp).
  if [ -d "$SRC/SoarUnitTests" ]; then
    cp -R "$SRC/SoarUnitTests" "$DEST/"
  fi

  # Core shared library
  # Windows: Soar.dll (bin/) + Soar.lib (lib/)
  # Unix:    libSoar.so or libSoar.dylib (lib/)
  cp "$SRC"/bin/Soar.dll "$DEST/" 2>/dev/null || true
  cp "$SRC"/lib/Soar.lib "$DEST/" 2>/dev/null || true
  find "$SRC/lib" -maxdepth 1 -name "libSoar.*" \
    -not -name "*.cmake" -exec cp -v {} "$DEST/" \;  2>/dev/null || true

  # Java JNI library
  # Unix: java/libJava_sml_ClientInterface.{so,jnilib}
  # Windows: java/Java_sml_ClientInterface.dll (after install fix)
  find "$SRC/java" -maxdepth 1 -name "*Java_sml_ClientInterface*" \
    -not -name "*.java" -not -name "*.jar" \
    -exec cp -v {} "$DEST/" \;  2>/dev/null || true

  # Python SWIG binding
  find "$SRC/python" -maxdepth 1 -name "_Python_sml_ClientInterface*" \
    -exec cp -v {} "$DEST/" \;  2>/dev/null || true
  cp "$SRC/python/Python_sml_ClientInterface.py" "$DEST/" 2>/dev/null || true

  # Tcl SWIG binding
  find "$SRC/tcl" -maxdepth 1 -name "*Tcl_sml_ClientInterface*" \
    -exec cp -v {} "$DEST/" \;  2>/dev/null || true
  cp "$SRC/tcl/pkgIndex.tcl" "$DEST/" 2>/dev/null || true

  # TclSoarLib — Tcl interpreter embedded in Soar (loaded at runtime)
  # Windows: tclsoarlib.dll (bin/), Unix: libtclsoarlib.{so,dylib} (lib/)
  cp "$SRC"/bin/tclsoarlib.dll "$DEST/" 2>/dev/null || true
  find "$SRC/lib" -maxdepth 1 -name "libtclsoarlib.*" \
    -exec cp -v {} "$DEST/" \;  2>/dev/null || true

  # C# SWIG binding
  find "$SRC/csharp" -maxdepth 1 -name "*CSharp_sml_ClientInterface*" \
    -exec cp -v {} "$DEST/" \;  2>/dev/null || true
  cp "$SRC/csharp/sml_csharp.dll" "$DEST/" 2>/dev/null || true

  # Platform-specific SWT jar
  cp "$SRC/java/swt.jar" "$DEST/" 2>/dev/null || true
done

# ── Cross-platform resources (from first available platform) ─────────────────

ANY=""
for PLAT in "${PLATFORM_IDS[@]}"; do
  if [ -d "$PLATFORMS_DIR/soar-${PLAT}" ]; then
    ANY="$PLATFORMS_DIR/soar-${PLAT}"
    break
  fi
done

if [ -z "$ANY" ]; then
  echo "ERROR: No platform builds found in $PLATFORMS_DIR"
  exit 1
fi

echo "=== Cross-platform resources (from $ANY) ==="

# Headers
mkdir -p "$SUITE/include"
cp -r "$ANY"/include/* "$SUITE/include/"

# TclSoarLib support scripts (cross-platform)
if [ -d "$ANY/tcl" ]; then
  mkdir -p "$SUITE/bin/tcl"
  cp "$ANY"/tcl/*.tcl "$SUITE/bin/tcl/" 2>/dev/null || true
  cp "$ANY"/tcl/*.soar "$SUITE/bin/tcl/" 2>/dev/null || true
  cp "$ANY"/tcl/tclIndex "$SUITE/bin/tcl/" 2>/dev/null || true
fi

# Java JARs
mkdir -p "$SUITE/bin/java"
for jar in sml.jar soar-debugger.jar soar-debugger-api.jar soar-smljava.jar \
           jcommon-1.0.10.jar jfreechart-1.0.6.jar jfreechart-1.0.6-swt.jar \
           swtgraphics2d.jar commons-cli-1.9.0.jar commons-logging-1.1.1.jar \
           log4j-1.2.15.jar stopwatch-0.4-with-deps.jar; do
  cp "$ANY/java/$jar" "$SUITE/bin/java/" 2>/dev/null || \
    echo "  note: $jar not found (optional)"
done

# SoarJavaDebugger launcher + settings
cp "$ANY/SoarJavaDebugger.jar" "$SUITE/bin/" 2>/dev/null || true
cp "$ANY/settings.soar" "$SUITE/bin/" 2>/dev/null || true

# ── External repos ───────────────────────────────────────────────────────────

if [ -n "$REPOS_DIR" ] && [ -d "$REPOS_DIR" ]; then
  echo "=== External repos ==="

  # Agents
  if [ -d "$REPOS_DIR/Agents" ]; then
    mkdir -p "$SUITE/Agents"
    for d in "$REPOS_DIR/Agents"/*/; do
      name=$(basename "$d")
      [ "$name" = ".git" ] && continue
      cp -r "$d" "$SUITE/Agents/$name"
    done
    cp "$REPOS_DIR/Release-Support/txt/Agents_readme.md" \
       "$SUITE/Agents/readme.md" 2>/dev/null || true
  fi

  # Eaters & TankSoar agents + configs
  if [ -d "$REPOS_DIR/Domains-Eaters-TankSoar" ]; then
    cp -r "$REPOS_DIR/Domains-Eaters-TankSoar/agents/eaters" \
       "$SUITE/Agents/eaters" 2>/dev/null || true
    cp -r "$REPOS_DIR/Domains-Eaters-TankSoar/agents/tanksoar" \
       "$SUITE/Agents/tanksoar" 2>/dev/null || true
    mkdir -p "$SUITE/bin/games/maps"
    cp "$REPOS_DIR/Domains-Eaters-TankSoar/config/"*.cnf \
       "$SUITE/bin/games/" 2>/dev/null || true
    cp -r "$REPOS_DIR/Domains-Eaters-TankSoar/config/maps/"* \
       "$SUITE/bin/games/maps/" 2>/dev/null || true
  fi

  # VisualSoar
  find "$REPOS_DIR/VisualSoar" -name "VisualSoar.jar" -path "*/build/libs/*" \
    -exec cp -v {} "$SUITE/bin/" \;  2>/dev/null || true
  # Jackson JARs from VisualSoar
  for jar in jackson-annotations jackson-core jackson-databind; do
    find "$REPOS_DIR/VisualSoar/lib" -name "${jar}*.jar" \
      -exec cp -v {} "$SUITE/bin/java/" \;  2>/dev/null || true
  done

  # Eaters_TankSoar.jar (built from Domains-Eaters-TankSoar via Ant)
  cp "$REPOS_DIR/Domains-Eaters-TankSoar/Eaters_TankSoar.jar" \
     "$SUITE/bin/" 2>/dev/null || true

  # Documentation
  mkdir -p "$SUITE/Documentation/SoarTutorial"
  cp "$REPOS_DIR/Release-Support/pdf/SoarManual.pdf" \
     "$SUITE/Documentation/" 2>/dev/null || true
  find "$REPOS_DIR/VisualSoar" -name "VisualSoar_UsersManual.pdf" \
    -exec cp -v {} "$SUITE/Documentation/" \;  2>/dev/null || true
  find "$REPOS_DIR/Release-Support/pdf" -name "Soar Tutorial*" \
    -exec cp -v {} "$SUITE/Documentation/SoarTutorial/" \;  2>/dev/null || true

  # Launcher scripts
  for ext in bat sh command; do
    for script in SoarCLI SoarJavaDebugger VisualSoar Eaters TankSoar setup; do
      cp "$REPOS_DIR/Release-Support/scripts/${script}.${ext}" \
         "$SUITE/" 2>/dev/null || true
    done
  done

  # Root text files
  cp "$REPOS_DIR/Release-Support/txt/README.md" "$SUITE/" 2>/dev/null || true
  cp "$REPOS_DIR/Release-Support/txt/Building_Soar.md" "$SUITE/" 2>/dev/null || true
  cp "$REPOS_DIR/Release-Support/txt/license.txt" "$SUITE/" 2>/dev/null || true
  cp "$REPOS_DIR/Release-Support/txt/Release_Notes_${VERSION}.md" \
     "$SUITE/" 2>/dev/null || true
fi

# ── Finalize ──────────────────────────────────────────────────────────────────

# Set execute permissions on shell/command scripts
find "$SUITE" \( -name "*.sh" -o -name "*.command" \) -exec chmod +x {} \; 2>/dev/null || true
# Executables
find "$SUITE/bin" -maxdepth 2 -type f \
  \( -name "soar" -o -name "UnitTests" \) -exec chmod +x {} \; 2>/dev/null || true

# Create zip
zip -r "${SUITE}.zip" "$SUITE/"

echo ""
echo "=== Release assembly complete ==="
du -sh "${SUITE}.zip"
echo "Contents:"
find "$SUITE" -maxdepth 2 -type d | sort
