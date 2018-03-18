; ModuleID = 'example5.c'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %source = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  store i32 0, i32* %1
  %2 = load i32* %source, align 4
  store i32 %2, i32* %a, align 4
  %3 = load i32* %source, align 4
  store i32 %3, i32* %b, align 4
  br label %4

; <label>:4                                       ; preds = %18, %0
  %5 = load i32* %a, align 4
  %6 = load i32* %source, align 4
  %7 = icmp ne i32 %5, %6
  br i1 %7, label %8, label %20

; <label>:8                                       ; preds = %4
  %9 = load i32* %c, align 4
  store i32 %9, i32* %b, align 4
  %10 = load i32* %a, align 4
  %11 = load i32* %source, align 4
  %12 = icmp sgt i32 %10, %11
  br i1 %12, label %13, label %15

; <label>:13                                      ; preds = %8
  %14 = load i32* %d, align 4
  store i32 %14, i32* %b, align 4
  br label %18

; <label>:15                                      ; preds = %8
  %16 = load i32* %source, align 4
  store i32 %16, i32* %c, align 4
  %17 = load i32* %c, align 4
  store i32 %17, i32* %b, align 4
  br label %18

; <label>:18                                      ; preds = %15, %13
  %19 = load i32* %d, align 4
  store i32 %19, i32* %a, align 4
  br label %4

; <label>:20                                      ; preds = %4
  %21 = load i32* %d, align 4
  store i32 %21, i32* %c, align 4
  %22 = load i32* %b, align 4
  %23 = load i32* %d, align 4
  %24 = icmp slt i32 %22, %23
  br i1 %24, label %25, label %27

; <label>:25                                      ; preds = %20
  %26 = load i32* %c, align 4
  store i32 %26, i32* %b, align 4
  br label %29

; <label>:27                                      ; preds = %20
  %28 = load i32* %b, align 4
  store i32 %28, i32* %d, align 4
  br label %29

; <label>:29                                      ; preds = %27, %25
  %30 = load i32* %c, align 4
  store i32 %30, i32* %a, align 4
  %31 = load i32* %source, align 4
  store i32 %31, i32* %c, align 4
  %32 = load i32* %1
  ret i32 %32
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.2 (tags/RELEASE_352/final)"}
