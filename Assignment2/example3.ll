; ModuleID = 'example3.c'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  store i32 0, i32* %1
  store i32 0, i32* %d, align 4
  br label %2

; <label>:2                                       ; preds = %5, %0
  %3 = load i32* %d, align 4
  %4 = icmp sle i32 %3, 10
  br i1 %4, label %5, label %11

; <label>:5                                       ; preds = %2
  store i32 10, i32* %a, align 4
  %6 = load i32* %a, align 4
  %7 = load i32* %d, align 4
  %8 = add nsw i32 %6, %7
  store i32 %8, i32* %b, align 4
  %9 = load i32* %d, align 4
  %10 = add nsw i32 %9, 1
  store i32 %10, i32* %d, align 4
  br label %2

; <label>:11                                      ; preds = %2
  %12 = load i32* %a, align 4
  store i32 %12, i32* %c, align 4
  %13 = load i32* %1
  ret i32 %13
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.2 (tags/RELEASE_352/final)"}
