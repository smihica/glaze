;; *** OPERATORS ***

;;; util
(def difference (f lis1 lis2)
  (if lis1
      (if (mem (car lis1) lis2)
          (difference f (cdr lis1) lis2)
          (cons (car lis1)
                (difference f (cdr lis1) lis2)))))

(def intersection (f lis1 lis2)
  (if lis1
      (if (mem (car lis1) lis2)
          (cons (car lis1) (intersection f (cdr lis1) lis2))
          (intersection f (cdr lis1) lis2))))


(def compile-refer (x e next)
  (compile-lookup x e
                  [list 'refer-local _ next]
                  [list 'refer-free  _ next]
                  [list 'refer-global _ next]))

(def compile-lookup (x e return-local return-free return-global)
  (with (locals (car e) free (cdr e))
    (aif (pos x locals)
         (return-local it)
         (pos x free)
         (return-free it)
         (return-global x))))

(def find-dot-pos (lis)
  ((afn (lis x)
     (if (acons lis)
         (self (cdr lis) (+ 1 x))
         (if (no lis)
             -1
             x))) lis 0))

(def last-pair (x)
  (if (no (acons x))
      (error "last-pair: expects a list, given: " x)
      ((afn (x)
         (if (acons (cdr x))
             (self (cdr x))
             x)) x)))

(def dotted->proper (args)
  (if (is (type args) 'cons)
      (let lp (last-pair args)
        (if (and (is (type lp) 'cons) (no (cdr lp)))
            args
            (let copied (copy args)
              (let lpcpied (last-pair copied)
                (= (cdr lpcpied) (cons (cdr lp) nil))
                copied))))
      (cons args nil)))

;; generate boxing code (intersection of sets & vars)
;; if no need of boxing, just returns next
;;   sets(Set): assigned variables
;;   vars(List): used variables
;;   next(opc):
;;   ret(opc):
(def make-boxes (sets vars next)
  ((afn (vs)
     (if vs
         (list 'box (car vs) (self (cdr vs)))
         next))
   (intersection is sets vars)))


;; free: set
;; e: env(= [locals, frees])
;; next: opc
;; ret: opc["refer_*", n, ["argument",
;;           ["refer_*", n, ... ["argument", next]
(def collect-free (free e next)
  ((afn (f x)
     (if f
         (self (cdr f) (compile-refer (car f) e (list 'argument x)))
         x))
   free next))

(def transform-internal-def (body) body)

(def compile-fn (x e s f next)
  (if (< (len x) 2)
      (error "fn: expects at least 2, given " (len x))
      (with (args (cadr x) body (cddr x))
        (let tbody (transform-internal-def body)
          (let cbody `(do ,@tbody)
            (with (dotpos (find-dot-pos args)
                   proper (dotted->proper args))
              (with (free    (find-free cbody proper f)
                     sets    (find-sets cbody proper))
                (let do-body (compile cbody
                                      (cons proper free)
                                      (union is sets (intersection is s free))
                                      (union is f proper)
                                      '(return))
                  (let do-close (list
                                  'close
                                  (len free)
                                  (make-boxes sets proper do-body)
                                  next
                                  dotpos)

                    (collect-free free e do-close))))))))))


;1 (fn (a b c)
;2   (assign a 'x)
;3   (fn (d e f b)
;4     (assign d 'y)
;5     (+ a b)
;6     (fn (f g)
;7       (+ a c)
;8       (assign e 'z)
;9       (assign f 'a)
;10      (assign g 'b))))

;; 3の関数に着目する。
;; localsは(d e b)
;; outer-localsは(a b c)
;; freeは -> (a c) ( このバージョンだと a b c になる。バグ？ )
;; setsは -> (d e)
;; compilerに渡されるのは
;; x -> 4行目以降
;; e -> ((d e f b) . (a c)) このバージョンだと ((d e f b) . (a b c))
;; s -> (d e a)
;; f -> (a b c d e f)
;; next -> '(return)
;; となる。


;; find_free(): find free variables in x
;;              these variables are collected by collect_free().
;; x: expression
;; b: set of local vars (= variables which are not free)
;; f: set of free var candidates
;;    (local vars of outer lambdas)
;; ret: set of free vars
;; find-free は x に founction-body と
;; b に arguments を渡される。
;; f に 外側のlambdaのargumetsを渡される。
;; 内部の変数シンボルが外側のlambdaのargumentsを指し示す場合はそれが返される。
(def find-free (x b f)
  (if
    (asym x)
    (if (find x f)
        (list x))

    (acons x)
    (let first (car x)
      (if

        (is first 'do)
        (find-free (cdr x) b f)

        (is first 'quote)
        nil

        (is first 'fn)
        (with (vars (dotted->proper (cadr x))
               body (cddr x))
          (find-free body (union is b vars) f))

        (is first '%%%-scm-if)
        (with (testc (cadr x)
               thenc (cadr (cdr x))
               elsec (cadr (cdr (cdr x))))
          (union is
                 (find-free testc b f)
                 (union is
                        (find-free thenc b f)
                        (find-free elsec b f))))

        (is first 'assign)
        (with (vari (cadr x)
               exp  (cadr (cdr x)))
          (if (find vari f)
              (cons vari (find-free exp b f))
              (find-free exp b f)))

        (is first 'ccc)
        (let exp (cadr x)
          (find-free exp b f))

        ((afn (p s)
           (if p
               (self (cdr p) (union is s (find-free (car p) b f)))
               s))
         x
         nil)))))


(def find-free-2 (x b f)
  (if
    (asym x)
    (if (and (find x f) (no (find x b)))
        (list x))

    (acons x)
    (let first (car x)
      (if

        (is first 'do)
        (find-free (cdr x) b f)

        (is first 'quote)
        nil

        (is first 'fn)
        (with (vars (dotted->proper (cadr x))
               body (cddr x))
          (find-free body (union is b vars) f))

        (is first '%%%-scm-if)
        (with (testc (cadr x)
               thenc (cadr (cdr x))
               elsec (cadr (cdr (cdr x))))
          (union is
                 (find-free testc b f)
                 (union is
                        (find-free thenc b f)
                        (find-free elsec b f))))

        (is first 'assign)
        (with (vari (cadr x)
               exp  (cadr (cdr x)))
          (if (and (find vari f) (no (find vari b)))
              (cons vari (find-free exp b f))
              (find-free exp b f)))

        (is first 'ccc)
        (let exp (cadr x)
          (find-free exp b f))

        ((afn (p s)
           (if p
               (self (cdr p) (union is s (find-free (car p) b f)))
               s))
         x
         nil)))))


;; Enumerate variables which (could be assigned && included in v)
;; x: exp
;; v: set(vars)
;; ret: set
;; find-sets は x に founction-body と
;; v に arguments を渡される。
;; body を降下しながらassignされているargumentsを探す。
;; (fn ...) があったら、その中でのassignはその関数のargumentを抜かして考える。
(def find-sets (x v)
  (if
    (asym x)
    nil

    (acons x)
    (let first (car x)
      (if
        (is first 'do)
        (find-sets (cdr x) v)

        (is first 'quote)
        nil

        (is first 'fn)
        (with (vars (cadr x)
               body (cddr x))
          (find-sets body (difference is v (dotted->proper vars))))

        (is first '%%%-scm-if)
        (with (testc (cadr x)
               thenc (cadr (cdr x))
               elsec (cadr (cdr (cdr x))))
          (union is (find-sets testc v)
                 (union is
                        (find-sets thenc v)
                        (find-sets elsec v))))

        (is first 'assign)
        (with (vari (cadr x)
               exp  (cadr (cdr x)))
          (if (find vari v)
              (cons vari (find-sets exp v))
              (find-sets exp v)))

        (is first 'ccc)
        (find-sets (cadr x) v)

        ((afn (x s)
           (if x
               (self (cdr x) (union is (find-sets (car x) v) s))
               s))
         x nil)))))

(def is-tail? (n)
  (is (car n) 'return))

;; x: Expression  --- list of symbols or integer or ...
;; e: Environment --- (locals . frees)
;; s: Vars might be set
;; next: opc
;; ret:  opc
(def compile (x e s f next)
  (ccc
    (fn (return)
      (while
        t
        (if
          ;; x is a symbol !!
          (asym x)
          (return (compile-refer x e (if (find x s) (list 'indirect next) next)))

          ;; x is a cons !!
          (acons x)
          (let first (car x)
            (if
              ;; (%%%-scm-if ...)
              (is first '%%%-scm-if)
              (if (no (is (len x) 4))
                  (error "compile-error Invalid if: " x)
                  (with (testc (cadr x)
                         thenc (compile (cadr (cdr x)) e s f next)
                         elsec (compile (cadr (cdr (cdr x))) e s f next))
                    (= x testc)
                    (= next (list 'test thenc elsec))))

              ;; (assign ...)
              (is first 'assign)
              (do (= v (cadr x))
                  (= x (cadr (cdr x)))
                  (= next (compile-lookup
                            v e
                            [list 'assign-local _ next]
                            [list 'assign-free  _ next]
                            [list 'assign-global _ next])))

              ;; (do ...)
              (is first 'do)
              (return
                ((afn (x)
                   (if x
                       (compile (car x) e s f (self (cdr x)))
                       next))
                 (cdr x)))

              ;; (quote ...)
              (is first 'quote)
              (if (no (cdr x))
                  (error "compile-error Invalid quote: " x)
                  (return (list 'constant (cadr x) next)))

              ;; (fn ...)
              (is first 'fn)
              (return (compile-fn x e s f next))

              ;; (ccc ...)
              (is first 'ccc)
              (do
                (= x (cadr x))
                (let c (list
                         'conti
                         (if (is-tail? next) (+ (len (car e)) 1) 0)
                         (list 'argument
                               (list 'constant 1
                                     (list 'argument
                                           (compile x e s f
                                                    (if (is-tail? next)
                                                        (list 'shift 1 '(tco-hinted-apply))
                                                        '(apply)))))))

                  (return (if (is-tail? next) c (list 'frame c next)))))

              ;; other default
              (with (func (car x)
                     args (cdr x))

                (let c (compile
                         func e s f
                         (if (is-tail? next)
                             (list 'shift (len args) '(tco-hinted-apply))
                             '(apply)))

                  (= c
                     ;; push arguments reversed order.
                     ((afn (x c)
                        (if x
                            (self (cdr x) (compile (car x) e s f (list 'argument c)))
                            c))
                      args
                      ;; VM will push the number of arguments to the stack.
                      ; (compile (len args) e s f (list 'argument c))
                      (list 'argument-length (len args) c)
                      ))

                  (return (if (is-tail? next) c (list 'frame c next)))))))

          ;; not a cons. constant
          (return (list 'constant x next)))))))

(= macro-tables (list (table)))
(mac smac (name args . body)
  `(= ((car macro-tables) ',name)
      (fn ,args ,@body)))

(def smacex (exp binds)

  (if
    ;; exp is a sym
    (asym exp)
    (if (ssyntax exp)
        (smacex (ssexpand exp) binds)
        exp)

    ;; exp is a cons
    (acons exp)
    (let first (car exp)
      (if

        ;; (%%%-scm-if ...)
        (is first '%%%-scm-if)
        (list
          first
          (smacex (cadr exp) binds)
          (smacex (cadr (cdr exp)) binds)
          (smacex (cadr (cdr (cdr exp))) binds))

        ;; (assign ...)
        (is first 'assign)
        (list
          first
          (smacex (cadr exp) binds)
          (smacex (cadr (cdr exp)) binds))

        ;; (do ...)
        (is first 'do)
        (+ (list first)
           (map1 [smacex _ binds] (cdr exp)))

        ;; (quote ...)
        (is first 'quote)
        exp

        ;; (fn ...)
        (is first 'fn)
        (with (args (dotted->proper (cadr exp))
               body (cddr exp))
          (let newbinds (union is binds args)
            (+ (list first
                     (cadr exp))
               (map [smacex _ newbinds] body))))

        ;; other default
        (let firstex (smacex first binds)
          (aif (and (asym firstex)
                    (no (find firstex binds))
                    ((car macro-tables) firstex))
               (smacex (apply it (cdr exp)) binds)
               (+ (list firstex)
                  (map [smacex _ binds] (cdr exp)))))))

    ;; else constant.
    exp))

(smac if x
      (with (testc (car x)
             thenc (cadr x)
             elsec (cadr (cdr x))
             rest  (cdr (cdr (cdr x))))
        (if rest
            `(%%%-scm-if ,testc ,thenc (if ,@(cddr x)))
            `(%%%-scm-if ,testc ,thenc ,elsec))))

(smac let (b v . body)
      `(with (,b ,v) ,@body))

(smac with (vars . body)
      (with (vars (map car  (pair vars))
             vals (map cadr (pair vars)))
        `((fn ,vars ,@body) ,@vals)))

(smac def (name args . body)
      `(assign ,name (fn ,args (do ,@body))))

(smac mac (name args . body)
      `(error "sorry mac is not supported yet. use 'smac'."))


(= throw (fn (x) x))
(def error args
  (apply pr args)
  (pr "\n")
  (throw nil))

(def compile-top-level (exp)
  (ccc (fn (c)
         (= throw c)
         (let exp2 (smacex exp nil)
           (compile exp2 nil nil nil '(halt))))))

(mac record (vars val . exps)
  `(apply (fn ,vars ,@exps) ,val))

(mac record-case (target . conds)
  (w/uniq (target-s)
    `(let ,target-s ,target
       (if ,@(mappend
               (fn (c)
                 (let top (car c)
                   (if (or (is top nil) (is top 'else))
                       `((do ,@(cdr c)))
                       `((is (car ,target-s) ',top)
                         (record ,(cadr c) (cdr ,target-s) ,@(cddr c))))))
               conds)
           ))))

(def code-len (code)
  (record-case code
    (assign-local (n nex) (+ 2 (code-len nex)))
    (assign-free  (n nex) (+ 2 (code-len nex)))
    (assign-global (n nex) (+ 2 (code-len nex)))
    (refer-local (n nex)  (+ 2 (code-len nex)))
    (refer-free  (n nex)  (+ 2 (code-len nex)))
    (refer-global (n nex) (+ 2 (code-len nex)))
    (indirect     (nex)   (+ 1 (code-len nex)))
    (constant     (o nex) (+ 2 (code-len nex)))
    (argument-length (n nex) (+ 2 (code-len nex)))
    (test (thenc elsec)   (+ 2 (code-len thenc) (code-len elsec)))
    (box  (n nex)         (+ 2 (code-len nex)))
    (argument  (nex)      (+ 1 (code-len nex)))
    (close (x nex after dotpos) (+ 4 (code-len nex) (code-len after)))
    (frame (x nex)        (+ 1 (code-len x) (code-len nex)))
    (tco-hinted-apply ()  1)
    (apply ()             1)
    (shift (x nex)        (+ 2 (code-len nex)))
    (conti (x nex)        (+ 2 (code-len nex)))
    (return ()            1)
    (halt   ()            1)))

(def assembler (code)
  ((afn (i code)
    (record-case code
      (assign-local  (n nex) `((assign-local ,n) ,@(self (+ 1 i) nex)))
      (assign-free   (n nex) `((assign-free ,n) ,@(self (+ 1 i) nex)))
      (assign-global (n nex) `((assign-global ,n) ,@(self (+ 1 i) nex)))
      (refer-local  (n nex) `((refer-local ,n) ,@(self (+ 1 i) nex)))
      (refer-free   (n nex) `((refer-free ,n) ,@(self (+ 1 i) nex)))
      (refer-global (n nex) `((refer-global ,n) ,@(self (+ 1 i) nex)))
      (indirect     (nex)   `((indirect) ,@(self (+ 1 i) nex)))
      (constant     (o nex) `((constant ,o) ,@(self (+ 1 i) nex)))
      (argument-length (n nex) `((argument-length ,n) ,@(self (+ 1 i) nex)))
      (test (thenc elsec)   `((test ,(+ 2 (code-len thenc)))
                                 ,@(self (+ 1 i) thenc)
                                 ,@(self (+ 1 i (code-len thenc)) elsec)))
      (box (n nex)          `((box n) ,@(self (+ i 1) nex)))
      (argument (nex)       `((argument) ,@(self (+ i 1) nex)))
      (close (x nex after dotpos)
                            `((close ,x ,dotpos ,(+ 4 (code-len nex)))
                              ,@(self (+ i 1) nex)
                              ,@(self (+ i 1 (code-len nex)) after)))

      (frame (x nex)        `((frame ,(+ 2 (code-len x))) ,@(self (+ i 2) x) ,@(self (+ i 2 (code-len x)) nex)))
      (tco-hinted-apply ()  `((tco-hinted-apply)))
      (apply ()             `((apply)))
      (shift (x nex)        `((shift ,x) ,@(self (+ i 1) nex)))
      (conti (x nex)        `((conti ,x) ,@(self (+ i 1) nex)))
      (return ()            `((return)))
      (halt ()              `((halt))))) 0 code))

(def print-assembly (asm)
  (pr "(\n")
  (on i asm
    (pr i "\t;; " index "\n"))
  (pr ")\n"))

(def disps xs
  (if xs
      (do (disp (car xs))
          (if (cdr xs) (disp " "))
          (apply disps (cdr xs)))))

(def print-asm (asm i)

  (let print (rfn print (i . args)
               (if (is i 0)
                   (do (apply disps args)
                       (disp "\n"))
                   (do (disp "  ")
                       (apply print (- i 1) args))))

  (record-case asm
      (assign-local (n nex)
        (print i "ASSIGN_LOCAL" n)
        (print-asm nex i))
      (assign-free  (n nex)
        (print i "ASSIGN_FREE" n)
        (print-asm nex i))
      (assign-global (n nex)
        (print i "ASSIGN_GLOBAL" n)
        (print-asm nex i))
      (refer-local  (x nex)
        (print i "REFER_LOCAL" x)
        (print-asm nex i))
      (refer-free   (x nex)
        (print i "REFER_FREE" x)
        (print-asm nex i))
      (refer-global (x nex)
        (print i "REFER_GLOBAL" x)
        (print-asm nex i))
      (indirect     (nex)
        (print i "INDIRECT")
        (print-asm nex i))
      (constant     (o nex)
        (print i "CONSTANT" o)
        (print-asm nex i))
      (argument-length (n nex)
        (print i "ARGLEN" n)
        (print-asm nex i))
      (test         (thenc elsec)
        (print i "TEST")
        (print-asm thenc (+ i 1))
        (print-asm elsec (+ i)))
      (box          (n x)
        (print i "BOX" n)
        (print-asm x i))
      (argument     (nex)
        (print i "ARGUMENT")
        (print-asm nex i))
      (close        (x nex after dotpos)
        (print i "CLOSE" x dotpos)
        (print-asm nex (+ i 1))
        (print-asm after i))
      (frame        (x nex)
        (print i "FRAME")
        (print-asm x (+ i 1))
        (print-asm nex i))
      (tco-hinted-apply ()
        (print i "TCO_HINTED_APPLY"))
      (apply        ()
        (print i "APPLY"))
      (shift        (x nex)
        (print i "SHIFT" x)
        (print-asm nex i))
      (conti        (x nex)
        (print i "CONTI" x)
        (print-asm nex i))
      (return       ()
        (print i "RETURN"))
      (halt         ()
        (print i "HALT")))))


;; tests
(compile 'a (cons '(a) '()) '() '() '(halt))
(refer-local 0 (halt))

(compile 'a (cons '() '(a)) '() '() '(halt))
(refer-free 0 (halt))

(compile 'a (cons '(c b a) '()) '() '() '(halt))
(refer-local 2 (halt))

(compile 'a (cons '(a) '()) '(a) '() '(halt))
(refer-local 0 (indirect (halt)))

(assembler (compile-top-level '(if (- (+ 1 2) 1) a b)))

((frame 23)
 (constant 1)
 (argument)
 (frame 13)
 (constant 2)
 (argument)
 (constant 1)
 (argument)
 (argument-length 2)
 (refer-global +)
 (apply)
 (argument)
 (argument-length 2)
 (refer-global -)
 (apply)
 (test 5)
 (refer-global a)
 (halt)
 (refer-global b)
 (halt))

((frame 23)
 (constant 1)
 (argument)
 (frame 13)
 (constant 2)
 (argument)
 (constant 1)
 (argument)
 (argument-length 2)
 (refer-global +)
 (apply)
 (argument)
 (argument-length 2)
 (refer-global -)
 (apply)
 (halt))

((frame 13)
 (constant 2)
 (argument)
 (constant 1)
 (argument)
 (argument-length 2)
 (refer-global list)
 (apply)
 (halt))

(print-asm
  (compile-top-level
    '(def difference (f lis1 lis2)
       (if lis1
           (if (mem (car lis1) lis2)
               (difference f (cdr lis1) lis2)
               (cons (car lis1)
                     (difference f (cdr lis1) lis2)))))) 0)

(print-assembly
  (compile-top-level
    '(def difference (f lis1 lis2)
       (if lis1
           (if (mem (car lis1) lis2)
               (difference f (cdr lis1) lis2)
               (cons (car lis1)
                     (difference f (cdr lis1) lis2)))))))

(print-asm
  (compile-top-level
    '(def ccc-test (x)
       (ccc (fn (c)
              (c 'abc))))) 0)

(print-asm
  (compile-top-level
    '(def abc (x y . z)
       (+ x y (apply + z)))) 0)

(print-asm
  (compile-top-level
    '(assign
       x
       (fn (a b c)
         (assign a 'x)
         (fn (d e f b)
           (assign d 'y)
           (+ a b)
           (fn (f g)
             (+ a c)
             (assign e 'z)
             (assign f 'a)
             (assign g 'b)))))) 0)




(frame 
  (refer-local 2 (argument (frame (refer-local 1 (argument (constant 1 (argument (refer-global car (apply)))))) (argument (constant 2 (argument (refer-global mem (apply))))))))
  (test (refer-local 2 (argument (frame (refer-local 1 (argument (constant 1 (argument (refer-global cdr (apply)))))) (argument (refer-local 0 (argument (constant 3 (argument (refer-global difference (shift 3 (tco-hinted-apply))))))))))) (frame (refer-local 2 (argument (frame (refer-local 1 (argument (constant 1 (argument (refer-global cdr (apply)))))) (argument (refer-local 0 (argument (constant 3 (argument (refer-global difference (apply)))))))))) (argument (frame (refer-local 1 (argument (constant 1 (argument (refer-global car (apply)))))) (argument (constant 2 (argument (refer-global cons (shift 2 (tco-hinted-apply)))))))))))









(close 0 (refer-local 1 (test 
 (refer-global nil (return)))) (assign-global difference (halt)) -1)