; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)
@.str.l0 = private unnamed_addr constant [5 x i8] c"blob\00", align 1
@l0 = alias i8, getelementptr inbounds ([5 x i8], [5 x i8]* @.str.l0, i64 0, i64 0)

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

define double @Point_do(double %x) {
l1:
  %t0 = fadd double 0.000000e+00, 17.000000
  ; Load variable x
  %t1 = fadd double %t0, %x
  ret double %t1
}
%struct.Point2 = type { i8*, double, double }
define %struct.Point2* @Point2_constructor(i8* %x, double %y, double %z) {
  %heap_ptr = call i8* @malloc(i32 24)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point2*
  %x_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 0
  store i8* %x, i8** %x_ptr
  %y_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  %z_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 2
  store double %z, double* %z_ptr
  ret %struct.Point2* %obj_ptr
}

define double @Point2_do(double %x) {
l1:
  ; Load variable x
  %t0 = fadd double 0.000000e+00, 1.000000
  %t1 = fadd double %x, %t0
  ret double %t1
}

define double @main() {
l1:
  %t1 = fadd double 0.000000e+00, 1.000000
  %t0 = call %struct.Point* @Point_constructor(i8* @l0, double %t1)
  ; Variable assignment: a = %t0
  %t3_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 0
  %t3 = load i8*, i8** %t3_ptr
  %t2 = call double @prints(i8* %t3)
  %t4 = fadd double 0.000000e+00, 0.000000
  ret double %t4
}
