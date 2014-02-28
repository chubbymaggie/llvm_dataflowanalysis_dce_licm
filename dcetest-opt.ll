; ModuleID = 'dcetest-opt.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i386-pc-linux-gnu"

; Function Attrs: nounwind
define i32 @test1(i32 %t) #0 {
entry:
  ret i32 0
}

; Function Attrs: nounwind
define i32 @test2(i32 %t) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, %t
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 0
}

; Function Attrs: nounwind
define i32 @test3(i32 %t) #0 {
entry:
  %add = add nsw i32 %t, 1
  %add1 = add nsw i32 %add, 2
  %sub = sub nsw i32 %add1, 3
  %add2 = add nsw i32 %add1, %sub
  ret i32 %add2
}

; Function Attrs: nounwind
define i32 @test4(i32 %t) #0 {
entry:
  %add = add nsw i32 %t, 1
  %add1 = add nsw i32 %add, 2
  ret i32 %add1
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.4 (tags/RELEASE_34/final)"}
