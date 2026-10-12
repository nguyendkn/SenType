module sentype-diff/bamboo-run

go 1.18

require github.com/BambooEngine/bamboo-core v0.0.0

// bamboo-core (MIT) is cloned by tools/diff/run-diff.sh into build/ref; nothing is vendored.
replace github.com/BambooEngine/bamboo-core => ../../../build/ref/bamboo-core
