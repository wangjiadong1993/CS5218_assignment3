; ModuleID = 'example1.c'
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
  %2 = load i32* %c, align 4
  %3 = add nsw i32 %2, 1
  store i32 %3, i32* %b, align 4
  %4 = load i32* %d, align 4
  %5 = sub nsw i32 %4, 3
  store i32 %5, i32* %a, align 4
  %6 = load i32* %a, align 4
  %7 = load i32* %b, align 4
  %8 = add nsw i32 %6, %7
  %9 = load i32* %c, align 4
  %10 = load i32* %d, align 4
  %11 = add nsw i32 %9, %10
  %12 = icmp slt i32 %8, %11
  br i1 %12, label %13, label %18

; <label>:13                                      ; preds = %0
  %14 = load i32* %a, align 4
  %15 = icmp slt i32 %14, 10
  br i1 %15, label %16, label %18

; <label>:16                                      ; preds = %13
  %17 = load i32* %a, align 4
  store i32 %17, i32* %c, align 4
  br label %19

; <label>:18                                      ; preds = %13, %0
  store i32 10, i32* %c, align 4
  br label %19

; <label>:19                                      ; preds = %18, %16
  %20 = load i32* %b, align 4
  %21 = icmp sgt i32 %20, 0
  br i1 %21, label %22, label %23

; <label>:22                                      ; preds = %19
  store i32 -11, i32* %d, align 4
  br label %23

; <label>:23                                      ; preds = %22, %19
  %24 = load i32* %1
  ret i32 %24
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.2 (tags/RELEASE_352/final)"}
