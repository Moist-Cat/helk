; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)

%struct.Point = type { double, double }
define %struct.Point* @Point_constructor(double %x, double %y) {
  %heap_ptr = call i8* @malloc(i32 16)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point*
  %x_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 0
  store double %x, double* %x_ptr
  %y_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  ret %struct.Point* %obj_ptr
}

define double @Point_do(double %x) {
l0:
  %t0 = fadd double 0.000000e+00, 17.000000
  %t1 = fadd double %t0, %x
  ret double %t1
}

define double @Point_constant() {
l1:
  %t2 = fadd double 0.000000e+00, 1000.000000
  ret double %t2
}
%struct.Point2 = type { double, double, double }
define %struct.Point2* @Point2_constructor(double %x, double %y, double %z) {
  %heap_ptr = call i8* @malloc(i32 24)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point2*
  %x_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 0
  store double %x, double* %x_ptr
  %y_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  %z_ptr = getelementptr %struct.Point2, %struct.Point2* %obj_ptr, i32 0, i32 2
  store double %z, double* %z_ptr
  ret %struct.Point2* %obj_ptr
}

define double @Point2_do(double %x) {
l2:
  %t3 = fadd double 0.000000e+00, 1.000000
  %t4 = fadd double %x, %t3
  ret double %t4
}

define double @Point2_constant() {
l3:
  %t5 = fadd double 0.000000e+00, 1000.000000
  ret double %t5
}

define double @main() {
l4:
  %t7 = fadd double 0.000000e+00, 0.000000
  %t8 = fadd double 0.000000e+00, 0.000000
  %t6 = call %struct.Point* @Point_constructor(double %t7, double %t8)
  ; Variable assignment: a = %t6
  %t10 = fadd double 0.000000e+00, 0.000000
  %t11 = fadd double 0.000000e+00, 0.000000
  %t12 = fadd double 0.000000e+00, 0.000000
  %t9 = call %struct.Point2* @Point2_constructor(double %t10, double %t11, double %t12)
  ; Variable assignment: b = %t9
  %t14 = call double @Point_constant()
  %t13 = call double @print(double %t14)
  %t16 = call double @Point2_constant()
  %t15 = call double @print(double %t16)
  %t17 = fadd double 0.000000e+00, 0.000000
  ret double %t17
}
