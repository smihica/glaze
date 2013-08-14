(= debug-mode nil)

(def debug () (= debug-mode (no debug-mode)))

(mac reccase (expr . pats)
  (withs (p (split pats (- (len pats) 1))
          f (car p) l (cadr p))
    `(case (car ,expr)
       ,@(+ (mappend
              (fn (pat) `(,(car pat) (apply (fn ,@(cdr pat)) (cdr ,expr))))
              f)
            l))))

(def expand-macro (x)
  (case (type x)
    cons (reccase
           x
           (quote body `(quote ,@body))
           (fn (vars . body)
             `(fn ,vars ,(if (< (len body) 2)
                             (expand-macro (car body))
                             (expand-macro `(do ,@body)))))
           (do body
               `((fn ,(n-of (- (len body) 1) (uniq))
                   ,(expand-macro (last body)))
                 ,@(map expand-macro
                        (cut body 0 -1))))
           (map expand-macro x))
    x))

(def find-free (x b)
  (case (type x)
    sym  (if (no (mem x b)) (list x))
    cons (reccase
           x
           (quote body nil)
           (fn (vars body)
             (find-free body (union is vars b)))
           (if (test then else)
               (union is
                      (find-free test b)
                      (union is
                             (find-free then b)
                             (find-free else b))))
           (assign (var exp)
                   (union is
                          (if (no (mem var b)) (list var))
                          (find-free exp b)))

           (ccc (exp) (find-free exp b))

           ((afn (x)
              (if x
                  (union is
                         (find-free (car x) b)
                         (self (cdr x)))))
            x))))

(def set-minus (s1 s2)
  (if s1
      (if (mem (car s1) s2)
          (set-minus (cdr s1) s2)
          (cons (car s1) (set-minus (cdr s1) s2)))))

(def set-intersect (s1 s2)
  (if s1
      (if (mem (car s1) s2)
          (cons (car s1) (set-intersect (cdr s1) s2))
          (set-intersect (cdr s1) s2))))

(def find-sets (x v)
  (case (type x)
    cons (reccase
           x
           (quote (obj) nil)
           (fn (vars body)
             (find-sets body (set-minus v vars)))
           (if (test then else)
               (union is
                      (find-sets test v)
                      (union is
                             (find-sets then v)
                             (find-sets else v))))
           (assign (var x)
                   (union is
                          (if (mem var v) (list var))
                          (find-sets x v)))

           (ccc (exp) (find-sets exp v))

           (dedup (flat (map [find-sets _ v] x))))))

(def make-boxes (sets vars next)
  ((afn (vars n)
     (if vars
         (if (mem (car vars) sets)
             `(box ,n
                   ,(self (cdr vars) (+ n 1)))
             (self (cdr vars) (+ n 1)))
         next))
   vars 0))

(def tailp (next)
  (is (car next) 'return))

;; (find-free '(fn (a b c) ((fn (x y) (+ a b c d (- x y))) y z)) '())
;; -> (+ d - y z)
(def compile (x e s next)
  (when debug-mode
    (prn "x: " x)
    (prn "e: " e)
    (prn "s: " s)
    (prn "next: " next))
  (case (type x)
    sym  (compile-refer
           x e
           (if (mem x s)
               `(indirect ,next)
               next))
    cons (reccase
           x
           (quote (obj) `(constant ,obj ,next))

           (fn (vars body)
             (with (free (find-free body vars)
                    sets (find-sets body vars))
               (collect-free
                 free e
                 `(close ,(len free)
                         ,(make-boxes
                            sets (rev vars)
                            (compile body
                                     (cons (rev vars) free)
                                     (union is
                                            sets
                                            (set-intersect s free))
                                     `(return ,(+ (len vars) 1))))
                         ,next))))

           (if (test then else)
               (with (thenc (compile then e s next)
                      elsec (compile else e s next))
                 (compile test e s `(test ,thenc ,elsec))))

           (assign (var x)
                   (compile-lookup
                     var e
                     (fn (n)
                       (compile x e s `(assign-local ,n ,next)))
                     (fn (n)
                       (compile x e s `(assign-free ,n ,next)))))

           (ccc (x)
                (let c `(conti (argument
                                 (constant 1
                                           (argument
                                             ,(compile
                                                x e s
                                                (if (tailp next)
                                                    `(shift 1 ,(cadr next) (apply))
                                                    '(apply)))))))
                  (if (tailp next) c
                      `(frame ,next ,c))))

           ((afn (args c)
              (if (no args)
                  (if (tailp next)
                      c
                      `(frame ,next ,c))
                  (self (cdr args)
                        (compile (car args) e s `(argument ,c)))))
            (rev (+ (cdr x) (list (len (cdr x)))))
            (compile (car x) e s
                     (if (tailp next)
                         (list 'shift
                               (+ (len (cdr x)) 1)
                               (cadr next)
                               '(apply))
                         '(apply)))))

    `(constant ,x ,next)))

(def collect-free (vars e next)
  (if (no vars)
      next
      (collect-free
        (cdr vars) e
        (compile-refer
          (car vars) e
          (list 'argument next)))))

(def compile-refer (x e next)
  (compile-lookup
    x e
    (fn (n) `(refer-local ,n ,next))
    (fn (n) `(refer-free  ,n ,next))))

(def compile-lookup (x e return-local return-free)
  (aif (pos x (car e))
       (return-local it)
       (aif (pos x (cdr e))
            (return-free it)
            (err x "is not found."))))

(= stack (n-of 1000 0))

(def _push (x s)
  (= (stack s) x)
  (+ s 1))

(= primitives (list (cons 'nil nil)
                    (cons '+ +) (cons '- -) (cons '* *) (cons '/ /)
                    (cons '< <) (cons '> >) (cons '<= <=) (cons '>= >=)
                    (cons 'no no) (cons 'is is)
                    ))

(def index (s i)
  (let idx (- s i 1)
    (if (< idx 0)
        ((map cdr (rev primitives)) (- (abs idx)
                                       1 ;; indexize (1 -> 0)
                                       1 ;; ignore arg-len buffer.
                                       ))
        (stack idx))))

(def index-set (s i v)
  (= (stack (- (- s i) 1)) v))

(def range-get (from to)
  (map stack (range from to)))

(def show-stack (s)
  ((afn (i s)
     (when (< -1 i)
       (pr i ":" (let s (stack i)
                   ((afn (s)
                      (if (is (type s) 'cons)
                          (+ "(" (self (car s)) " ... )")
                          (tostring (pr s)))) s))
           (if (is s i) " (s)" "")
           "\n")
       (self (- i 1) s)))
   20 s))

(def closure (body n s)
  (let v (n-of (+ n 1) 0)
    (= (v 0) body)
    (for i 1 n (= (v i) (index s (- i 1))))
    v))

(def continuation (s)
  (closure `(refer-local 0 (nuate ,(save-stack s) (return 0))) 0 '()))
(def save-stack (s) (firstn s stack))
(def restore-stack (v)
  (= stack (+ v (nthcdr (len v) stack)))
  (len v))

(= step 0)
(def show-step (x)
  (do1
    (pr "step: " step " opcode: " (car x) "\n")
    (= step (+ step 1))))

(def box (x) (list x))
(def set-box (b x) (= (car b) x))
(def unbox (x) (car x))

(def closure-body  (c)   (c 0))
(def index-closure (c n) (c (+ n 1)))

(def shift-args (n m s)
  ((afn (i)
     (unless (< i 0)
       (index-set s (+ i m) (index s i))
       (self (- i 1))))
   (- n 1))
  (- s m))

(def vm (a x f c s)
  (if debug-mode
      (do
        (pr "a:" a "\n")
        (pr "x:" x "\n")
        (pr "f:" f "\n")
        (pr "c:" c "\n")
        (pr "s:" s "\n")
        (show-step x)
        (show-stack s)
        (aif (read)
             (case it
               q nil
               n (vm-tick a x f c s))))
      (vm-tick a x f c s)))

(def vm-tick (a x f c s)
  (reccase
    x
    (halt () a)
    (refer-local (n x)  (vm (index f (+ n 1)) x f c s))
    (refer-free  (n x)  (vm (index-closure c n) x f c s))
    (indirect    (x)    (vm (unbox a) x f c s))
    (constant (obj x)   (vm obj x f c s))
    (close (n body x)   (vm (closure body n s) x f c (- s n)))
    (box   (n x)        (do (index-set s (+ n 1)
                                       (box (index s (+ n 1))))
                            (vm a x f c s)))
    (test (then else)   (vm a (if a then else) f c s))
    (assign-local (n x) (do (set-box (index f (+ n 1)) a)
                            (vm a x f c s)))
    (assign-free  (n x) (do (set-box (index-closure c n) a)
                            (vm a x f c s)))
    (conti (x)          (vm (continuation s) x f c s))
    (nuate (stack x)    (vm a x f c (restore-stack stack)))
    (frame (ret x)      (vm a x f c (_push ret (_push f (_push c s)))))
    (argument (x)       (vm a x f c (_push a s)))
    (shift (n m x)      (vm a x f c (shift-args n m s)))
    (apply ()           (if (is (type a) 'cons)
                            (vm a (closure-body a) s a s)
                            (let var-len (index s 0)
                              (vm (apply a (range-get (- s 1 var-len)
                                                      (- s 2)))
                                  (index s (+ var-len 1))
                                  (index s (+ var-len 2))
                                  (index s (+ var-len 3))
                                  (- s
                                     var-len ;; vars
`                                     1       ;; var-len buffer
                                     1       ;; ret
                                     1       ;; f
                                     1       ;; c
                                     )))))
    (return (x)         (let s (- s x)
                          (vm a (index s 0) (index s 1) (index s 2) (- s 3))))
    (pr "Error: I don't know how to do. " x "\n")))


(def do-compile (x)
  (compile
    (expand-macro x)
    `(,(rev (map car primitives)))
    '()
    '(halt)))

(def evaluate (x)
  (= step 0)
  (= stack (n-of 1000 0))
  (vm '()
      (do-compile x)
      0
      '()
      (_push 0 0)))

(mac unit-tests tests
  `(do
     (= debug-mode nil)
     ,@(map (fn (x)
              `(if (is (evaluate ',x) ,x)
                   (pr "[PASS]: " ,x "\t" ',x "\n")
                   (pr "!!! FAIL: " ',x "\n")))
            tests)
     (= debug-mode t)
     nil))

(def run-tests ()
  (unit-tests
    ;; value
    1
    20

    ;; exp
    (+ 1 2)
    (+ (/ 1 2) 3)
    (- 1 2 3)
    (+ 1 (+ 2 3))
    (+ (- 3 4) (+ 1 (- 1 (* 3 2))))

    ;; fn
    ((fn (a b) (+ a b)) 10 20)
    (if 'a 1 2)
    (if nil 1 2)

    ((fn (c d)
       (* c
          ((fn (a b) (+ (- d c) a b d))
           d (+ d d))))
     10 3)

    ;; multiple exp
    ((fn (a b)
       (+ a b)
       (- a b)
       (/ a b))
     20 10)

    ;; do
    ((fn (a b)
       (do (+ 1 2)
           (+ a b)))
     10 20)

    ;; ccc
    (+ 1 (ccc (fn (c) (+ 3 5) (c (* 8 3)))))

    ((fn (x)
       ((fn (cc)
          (assign x (+ (ccc (fn (c) (assign cc c) (c 1))) x))
          (if (< x 4) (cc 2) x))
        nil))
     0)

    ;; ng
    ;((fn (x cc)
    ;(assign x (+ x (ccc (fn (c) (assign cc c) (c 1)))))
    ;(if (< x 10)
    ;(cc 2)
    ;x))
    ;0 nil)

    ;; assign-local
    ((fn (a b)
       (assign a 10)
       (* a b))
     1 3)

    ((fn (a b)
       (assign b 30)
       (* a b))
     1 3)

    ((fn (a b)
       (assign a 10)
       (assign b 30)
       (* a b))
     1 3)

    ;; assign-free
    ((fn (a b)
       ((fn ()
          (assign a 10)))
       (* a b))
     1 3)

    ;; assingn mixed
    ((fn (a)
       ((fn (b)
          ((fn (c d)
             (assign a 100)
             ((fn (e)
                (assign e (+ a e))
                (assign c 20)
                (+ a b c d e))
              5))
           3 4))
        2))
     1)

    ))


; preproc --------------------------------------------------------------

(def preproc (x i)
  (when debug-mode
    (prn x))
  (reccase
    x
    (frame (ret x)
           (let body (preproc x (+ i 1))
             `((frame ,(+ (len body) 1))
               ,@body
               ,@(preproc ret (+ i (len body) 1)))))

    (close (n b x)
           (let body (preproc b (+ i 1))
             `((close ,n ,(+ (len body) 1))
               ,@body
               ,@(preproc x (+ i (len body) 1)))))

    (test (th else)
          (let then (preproc th (+ i 1))
            `((test ,(+ (len then) 1))
              ,@then
              ,@(preproc else (+ i (len then) 1)))))

    (conti (x)
           `((conti) ,@(preproc x (+ i 1))))

    (shift (n m x) `((shift ,n ,m)
                     ,@(preproc x (+ i 1))))

    (constant (obj x)
              `((constant ,obj)
                ,@(preproc x (+ i 1))))

    (argument (x)
              `((argument)
                ,@(preproc x (+ i 1))))

    (refer-local (n x)
      `((refer-local ,n)
        ,@(preproc x (+ i 1))))

    (refer-free  (n x)
      `((refer-free ,n)
        ,@(preproc x (+ i 1))))

    (assign-local (n x)
                  `((assign-local ,n)
                    ,@(preproc x (+ i 1))))

    (assign-free  (n x)
                  `((assign-free  ,n)
                    ,@(preproc x (+ i 1))))

    (box         (n x)
      `((box ,n)
        ,@(preproc x (+ i 1))))

    (indirect (x)
              `((indirect)
                ,@(preproc x (+ i 1))))

    (apply () `((apply)))

    (return (x) `((return ,x)))

    (halt ()  `((halt)))

    nil))

;; vm -----------------------------------------------------------

(def closure2 (x p n s)
  (let v (n-of (+ n 2) 0)
    (= (v 0) x)
    (= (v 1) p)
    (for i 0 (- n 1) (= (v (+ i 2)) (index s i)))
    v))
(def closure-body2 (c) (c 0))
(def closure-pc2   (c) (c 1))
(def index-closure2 (c n) (c (+ n 2)))
(def continuation2 (s)
  (closure2 `((refer-local 0) (nuate ,(save-stack s)) (return 0)) 0 0 '()))

(def vm2 (x p a f c s)
  (if debug-mode
      (do
        (prn "x:" x)
        (prn "p:" p)
        (prn "a:" a)
        (prn "f:" f)
        (prn "c:" c)
        (prn "s:" s)
        (show-step x)
        (show-stack s)
        (aif (read)
             (case it
               q nil
               n (vm-tick2 x p a f c s))))
      (vm-tick2 x p a f c s)))

(def vm-tick2 (x p a f c s)
  (let op (x p)
    (reccase
      op
      (halt         ()      a)
      (refer-local  (n)     (vm2 x (inc p) (index f (+ n 1))   f c s))
      (refer-free   (n)     (vm2 x (inc p) (index-closure2 c n) f c s))
      (indirect     ()      (vm2 x (inc p) (unbox a)           f c s))
      (constant     (obj)   (vm2 x (inc p) obj                 f c s))
      (close        (n b)   (vm2 x (+ p b) (closure2 x (+ p 1) n s) f c (- s n)))
      (box          (n)     (do (index-set s (+ n 1) (box (index s (+ n 1))))
                                (vm2 x (inc p) a f c s)))
      (test         (n)     (vm2 x (if a (inc p) (+ p n)) a f c s))
      (assign-local (n)     (do (set-box (index f (+ n 1)) a)
                                (vm2 x (inc p) a f c s)))
      (assign-free  (n)     (do (set-box (index-closure2 c n) a)
                                (vm2 x (inc p) a f c s)))
      (frame        (ret)   (vm2 x (inc p) a f c (_push (list x (+ p ret))
                                                        (_push f
                                                               (_push c s)))))
      (argument     ()      (vm2 x (inc p) a f c (_push a s)))
      (shift        (n m)   (vm2 x (inc p) a f c (shift-args n m s)))
      (apply        ()      (if (is (type a) 'cons)
                                (vm2 (closure-body2 a) (closure-pc2 a) a s a s)
                                (let var-len (index s 0)
                                  (let xp (index s (+ var-len 1))
                                    (vm2 (car xp)
                                         (cadr xp)
                                         (apply a (range-get (- s 1 var-len)
                                                             (- s 2)))
                                         (index s (+ var-len 2))
                                         (index s (+ var-len 3))
                                         (- s var-len 4))))))
      (return (n)         (let ns (- s n)
                            (let xp (index ns 0)
                              (vm2 (car xp)
                                   (cadr xp)
                                   a
                                   (index ns 1)
                                   (index ns 2)
                                   (- ns 3)))))

      (conti ()           (vm2 x (inc p) (continuation2 s) f c s))
      (nuate (stack)      (vm2 x (inc p) a f c (restore-stack stack)))

      (prn "Error: I don't know how to do. "))))

(def evaluate2 (x)
  (= step 0)
  (= stack (n-of 1000 0))
  (vm2 (preproc (do-compile x) 0)
       0
       '()
       0
       '()
       (_push 0 0)))

;; tester -----------------------------------------------------

(mac unit-tests2 tests
  `(do
     (= debug-mode nil)
     ,@(map (fn (x)
              `(if (is (evaluate2 ',x) ,x)
                   (pr "[PASS]: " ,x "\t" ',x "\n")
                   (pr "!!! FAIL: " ',x "\n")))
            tests)
     (= debug-mode t)
     nil))

(unit-tests2
    ;; value
    1
    20

    ;; exp
    (+ 1 2)
    (+ (/ 1 2) 3)
    (- 1 2 3)
    (+ 1 (+ 2 3))
    (+ (- 3 4) (+ 1 (- 1 (* 3 2))))

    ;; fn
    ((fn (a b) (+ a b)) 10 20)
    (if 'a 1 2)
    (if nil 1 2)

    ((fn (c d)
       (* c
          ((fn (a b) (+ (- d c) a b d))
           d (+ d d))))
     10 3)

    ;; multiple exp
    ((fn (a b)
       (+ a b)
       (- a b)
       (/ a b))
     20 10)

    ;; do
    ((fn (a b)
       (do (+ 1 2)
           (+ a b)))
     10 20)

    ;; ccc
    (+ 1 (ccc (fn (c) (+ 3 5) (c (* 8 3)))))

    ;; ng
    ((fn (x)
       ((fn (cc)
          (assign x (+ (ccc (fn (c) (assign cc c) (c 1))) x))
          (if (< x 4) (cc 2) x))
        nil))
     0)

    ;; ng
    ;((fn (x cc)
    ;(assign x (+ x (ccc (fn (c) (assign cc c) (c 1)))))
    ;(if (< x 10)
    ;(cc 2)
    ;x))
    ;0 nil)

    ;; assign-local
    ((fn (a b)
       (assign a 10)
       (* a b))
     1 3)

    ((fn (a b)
       (assign b 30)
       (* a b))
     1 3)

    ((fn (a b)
       (assign a 10)
       (assign b 30)
       (* a b))
     1 3)

    ;; assign-free
    ((fn (a b)
       ((fn ()
          (assign a 10)))
       (* a b))
     1 3)

    ;; assingn mixed
    ((fn (a)
       ((fn (b)
          ((fn (c d)
             (assign a 100)
             ((fn (e)
                (assign e (+ a e))
                (assign c 20)
                (+ a b c d e))
              5))
           3 4))
        2))
     1)

    )

(= debug-mode nil)

(def asm-string (codes)
  (tostring
    (each code codes
      (each c (intersperse " " code) (pr c))
      (prn))))

(defop arc-compiler req
       (pr (asm-string (preproc (do-compile (read (alref (req 'args) "code"))) 0))))

(serve 8800)
