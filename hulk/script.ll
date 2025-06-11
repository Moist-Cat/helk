; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @pow(double, double)
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

define double @Point_do(double %self, double %x) {
l0:
  %t0 = fadd double 0.000000e+00, 17.000000
  ; Load variable x
  %t1 = fadd double %t0, %x
  ret double %t1
}

define double @Point_constant(%struct.Point2* %self) {
l0:
  %t0 = fadd double 0.000000e+00, 1000.000000
  ret double %t0
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

define double @Point2_do(%struct.Point2* %self, double %x) {
l0:
  %t0_ptr = getelementptr %struct.Point2, %struct.Point2* %self, i32 0, i32 2
  %t0 = load double, double* %t0_ptr
  ; Load variable x
  %t1 = fadd double %t0, %x
  ret double %t1
}

define double @Point2_constant(%struct.Point2* %self) {
l0:
  %t0 = fadd double 0.000000e+00, 1000.000000
  ret double %t0
}

define double @main() {
l0:
  %t1 = fadd double 0.000000e+00, 0.000000
  %t2 = fadd double 0.000000e+00, 0.000000
  %t0 = call %struct.Point* @Point_constructor(double %t1, double %t2)
  ; Variable assignment: a = %t0
  %t4 = fadd double 0.000000e+00, 1.000000
  %t5 = fadd double 0.000000e+00, 0.000000
  %t6 = fadd double 0.000000e+00, 0.000000
  %t3 = call %struct.Point2* @Point2_constructor(double %t4, double %t5, double %t6)
  ; Variable assignment: b = %t3
  ; Load variable b
  %t8 = call double @Point2_constant(%struct.Point2* %t3)
  %t7 = call double @print(double %t8)
  ; Load variable b
  %t11 = fadd double 0.000000e+00, 3.000000
  %t10 = call double @Point2_do(%struct.Point2* %t3, double %t11)
  %t9 = call double @print(double %t10)
  %t12 = fadd double 0.000000e+00, 0.000000
  ret double %t12
}
