// Bamboo (bamboo-core engine, MIT) side of the differential run.
// Input lines: <label>\t<T|V>\t<keys>; output: the text after typing the keys and a word break.
// Flags: free tone marking ON, auto-correct ON, EstdToneStyle OFF (that flag selects the old
// "hòa" placement; cleared it gives the modern "hoà" SenType uses by default). A word that is
// not valid Vietnamese is restored to the keys typed, as ibus/fcitx5-bamboo do on word break.
package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	bamboo "github.com/BambooEngine/bamboo-core"
)

func typeWord(im bamboo.InputMethod, word string) string {
	e := bamboo.NewEngine(im, bamboo.EfreeToneMarking|bamboo.EautoCorrectEnabled)
	e.ProcessString(word, bamboo.VietnameseMode)
	if !e.IsValid(true) {
		return e.GetProcessedString(bamboo.EnglishMode)
	}
	return e.GetProcessedString(bamboo.VietnameseMode)
}

func main() {
	telex := bamboo.ParseInputMethod(bamboo.InputMethodDefinitions, "Telex")
	vni := bamboo.ParseInputMethod(bamboo.InputMethodDefinitions, "VNI")
	in := bufio.NewScanner(os.Stdin)
	in.Buffer(make([]byte, 1<<20), 1<<20)
	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()
	for in.Scan() {
		parts := strings.SplitN(in.Text(), "\t", 3)
		if len(parts) < 3 {
			continue
		}
		im := telex
		if parts[1] == "V" {
			im = vni
		}
		words := strings.Split(parts[2], " ")
		for i, w := range words {
			words[i] = typeWord(im, w)
		}
		fmt.Fprintln(out, strings.Join(words, " "))
	}
}
