; ModuleID = 'memelang'
declare double @max(double, double)
declare double @min(double, double)
declare double @print(double)
declare double @prints(i8* nocapture) nounwind
declare i8* @malloc(i32)
declare void @free(i8*)

%struct.Point = type { double, double }
define %struct.Point* @Point_constructor(double %x, double %y) {
  %heap_ptr = call i8* @malloc(i32 0)
  %obj_ptr = bitcast i8* %heap_ptr to %struct.Point*
  %x_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 0
  store double %x, double* %x_ptr
  %y_ptr = getelementptr %struct.Point, %struct.Point* %obj_ptr, i32 0, i32 1
  store double %y, double* %y_ptr
  ret %struct.Point* %obj_ptr
}

define double @Point_do() {
l0:
  %t0 = fadd double 0.000000e+00, 17.000000
  ret double %t0
}

define double @main() {
l1:
  %t2 = fadd double 0.000000e+00, 3.000000
  %t3 = fadd double 0.000000e+00, 4.000000
  %t1 = call %struct.Point* @Point_constructor(double %t2, double %t3)
  ; Variable assignment: a = %t1
  %t5_ptr = getelementptr %struct.Point, %struct.Point* %t1, i32 0, i32 0
  %t5 = load double, double* %t5_ptr
  %t4 = call double @print(double %t5)
  ; Variable assignment: c = %t4
  %t7_ptr = getelementptr %struct.Point, %struct.Point* %t1, i32 0, i32 1
  %t7 = load double, double* %t7_ptr
  %t6 = call double @print(double %t7)
  ; Variable assignment: d = %t6
  %t9_ptr = getelementptr %struct.Point, %struct.Point* %t1, i32 0, i32 1
  %t9 = load double, double* %t9_ptr
  %t8 = call double @print(double %t9)
  ; Variable assignment: e = %t8
  %t11_ptr = getelementptr %struct.Point, %struct.Point* %t1, i32 0, i32 0
  %t11 = load double, double* %t11_ptr
  %t10 = call double @print(double %t11)
  ; Variable assignment: f = %t10
  %t12 = fadd double 0.000000e+00, 0.000000
  %t14 = fadd double 0.000000e+00, 1.000000
  %t15 = fadd double 0.000000e+00, 1.000000
  %t13 = call %struct.Point* @Point_constructor(double %t14, double %t15)
  ; Variable assignment: b = %t13
  %t17 = call double @Point_do()
  %t16 = call double @print(double %t17)
  %t18 = fadd double 0.000000e+00, 0.000000
  ret double %t18
}
