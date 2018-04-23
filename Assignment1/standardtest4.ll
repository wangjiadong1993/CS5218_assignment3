; ModuleID = 'standardtest4.c'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  %e = alloca i32, align 4
  %sink = alloca i32, align 4
  %source = alloca i32, align 4
  %i = alloca i32, align 4
  %N = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 0, i32* %1
  store i32 1234, i32* %source, align 4
  store i32 0, i32* %i, align 4
  store i32 50, i32* %N, align 4
  br label %2

; <label>:2                                       ; preds = %21, %0
  %3 = load i32* %i, align 4
  %4 = load i32* %N, align 4
  %5 = icmp slt i32 %3, %4
  br i1 %5, label %6, label %25

; <label>:6                                       ; preds = %2
  %7 = load i32* %a, align 4
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %9, label %11

; <label>:9                                       ; preds = %6
  store i32 5, i32* %c, align 4
  %10 = load i32* %c, align 4
  store i32 %10, i32* %d, align 4
  br label %12

; <label>:11                                      ; preds = %6
  store i32 12, i32* %d, align 4
  br label %12

; <label>:12                                      ; preds = %11, %9
  %13 = load i32* %source, align 4
  store i32 %13, i32* %e, align 4
  %14 = load i32* %b, align 4
  %15 = icmp sgt i32 %14, 0
  br i1 %15, label %16, label %18

; <label>:16                                      ; preds = %12
  %17 = load i32* %d, align 4
  store i32 %17, i32* %z, align 4
  br label %21

; <label>:18                                      ; preds = %12
  %19 = load i32* %e, align 4
  store i32 %19, i32* %y, align 4
  %20 = load i32* %y, align 4
  store i32 %20, i32* %z, align 4
  br label %21

; <label>:21                                      ; preds = %18, %16
  store i32 5, i32* %z, align 4
  %22 = load i32* %z, align 4
  store i32 %22, i32* %sink, align 4
  %23 = load i32* %i, align 4
  %24 = add nsw i32 %23, 1
  store i32 %24, i32* %i, align 4
  br label %2

; <label>:25                                      ; preds = %2
  %26 = load i32* %1
  ret i32 %26
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.2 (tags/RELEASE_352/final)"}
