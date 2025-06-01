; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)

%struct.Point = type { double, double }
define %struct.Point* @Point_constructor(double %x, double %y) {
  %heap_ptr = call i8* @malloc(i32 18762144)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point*
  %x_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 0
  store double %x, double* %x_ptr
  %y_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  ret %struct.Point* %obj_ptr
}

define double @main() {
l0:
  %t1 = fadd double 0.000000e+00, 3.000000
  %t2 = fadd double 0.000000e+00, 4.000000
  %t0 = call %struct.Point* @Point_constructor(double %t1, double %t2)
  ; Variable assignment: a = %t0
  %t4_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 1
  %t4 = load double, double* %t4_ptr
  %t3 = call double @print(double %t4)
  %t6_ptr = getelementptr %struct.Point, %struct.Point* %t0, i32 0, i32 0
  %t6 = load double, double* %t6_ptr
  %t5 = call double @print(double %t6)
  %t7 = fadd double 0.000000e+00, 0.000000
  ret double %t7
}
