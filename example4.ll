; ModuleID = 'example4.c'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  %source = alloca i32, align 4
  store i32 0, i32* %1
  %2 = load i32* %source, align 4
  store i32 %2, i32* %a, align 4
  %3 = load i32* %source, align 4
  store i32 %3, i32* %b, align 4
  %4 = load i32* %source, align 4
  store i32 %4, i32* %c, align 4
  %5 = load i32* %source, align 4
  store i32 %5, i32* %d, align 4
  %6 = load i32* %a, align 4
  %7 = icmp sgt i32 %6, 0
  br i1 %7, label %8, label %9

; <label>:8                                       ; preds = %0
  store i32 2, i32* %b, align 4
  store i32 4, i32* %d, align 4
  br label %10

; <label>:9                                       ; preds = %0
  store i32 3, i32* %c, align 4
  br label %10

; <label>:10                                      ; preds = %9, %8
  %11 = load i32* %source, align 4
  store i32 %11, i32* %b, align 4
  %12 = load i32* %1
  ret i32 %12
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.2 (tags/RELEASE_352/final)"}
