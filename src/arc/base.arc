(def accumulate (op initial sequence)
  (if (no sequence)
      initial
      (op (car sequence)
          (accumulate op initial (cdr sequence)))))

(def append (seq1 seq2)
  (accumulate cons seq2 seq1))

(def reduce (f lists)
  (def iter (f rest acc)
    (if (no rest) acc
        (iter f (cdr rest) (f acc (car rest)))))
  (iter f (cdr lists) (car lists)))

(def join lists
  (reduce append lists))

(def map1 (f xs)
  (if xs
      (cons (f (car xs)) (map1 f (cdr xs)))))

(def pair (xs . f)
  (if (no xs)
      nil
      (no (cdr xs))
      (list (list (car xs)))
      ((fn (f) (cons ((if f f list) (car xs) (cadr xs))
                     (pair (cddr xs) f))) (car f))))

(mac with (params . body)
  `((fn ,(map1 car (pair params)) ,@body)
    ,@(map1 cadr (pair params))))

(mac let (var val . body)
  `(with (,var ,val)
     ,@body))

;; TODO
;; = is place function.
(mac = (var val)
  (list 'set var val))

;; TODO
;; append -> +
(mac w/uniq (args . body)
  `(with ,(reduce append (map1 (fn (arg) `(,arg (uniq))) args))
     ,@body))

;; (aif (> 1 2) (+ it 1) 42 (+ it 2))
(mac aif exps
  `(let it ,(car exps)
     (if it
         ,(cadr exps)
         ,(if (nthcdr 3 exps)
              `(aif ,@(nthcdr 2 exps))
              (nth 2 exps)))))

(def isa (a b) (is (type a) b))