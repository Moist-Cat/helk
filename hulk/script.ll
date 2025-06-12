; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)
@.str.l0 = private unnamed_addr constant [2 x i8] c"1\00", align 1
@l0 = alias i8, getelementptr inbounds ([2 x i8], [2 x i8]* @.str.l0, i64 0, i64 0)
@.str.l1 = private unnamed_addr constant [2 x i8] c"3\00", align 1
@l1 = alias i8, getelementptr inbounds ([2 x i8], [2 x i8]* @.str.l1, i64 0, i64 0)

%struct.Point = type { i8*, double }
define %struct.Point* @Point_constructor(i8* %x, double %y) {
  %heap_ptr = call i8* @malloc(i32 16)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point*
  %x_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 0
  store i8* %x, i8** %x_ptr
  %y_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  ret %struct.Point* %obj_ptr
}

define i8* @Point_do(%struct.Point* %self, i8* %x) {
l2:
  %t0_ptr = getelementptr %struct.Point, %struct.Point* %self, i32 0, i32 0
  %t0 = load i8*, i8** %t0_ptr
  ret i8* %t0
}

define double @main() {
l2:
  %t1 = fadd double 0.000000e+00, 1.000000
  %t0 = call %struct.Point* @Point_constructor(i8* @l0, double %t1)
  ; Variable assignment: b = %t0
  ; Load variable b
  %t3 = call i8* @Point_do(%struct.Point* %t0, i8* @l1)
  %t2 = call double @prints(i8* %t3)
  %t4 = fadd double 0.000000e+00, 0.000000
  ret double %t4
}
