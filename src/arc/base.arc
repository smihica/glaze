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

(def cadr (x) (car (cdr x)))
(def cddr (x) (cdr (cdr x)))

(def caris (x y)
  (and (acons x)
       (is (car x) y)))

(def pair (xs)
  (if (no xs)
       nil
      (no (cdr xs))
       (list (list (car xs)))
      (cons (list (car xs) (cadr xs))
            (pair (cddr xs)))))

(def %qq-expand (x)
  (if (caris x 'unquote)
      (cadr x)

      (caris x 'unquote-splicing)
      (err "Illegal")

      (caris x 'quasiquote)
      (%qq-expand
        (%qq-expand (cadr x)))

      (acons x)
      (list 'join
            (%qq-expand-list (car x))
            (%qq-expand (cdr x)))

      (list 'quote x)))

(def %qq-expand-list (x)
  (if (caris x 'unquote)
      (list 'list (cadr x))

      (caris x 'unquote-splicing)
      (cadr x)

      (caris x 'quasiquote)
      (%qq-expand-list (%qq-expand (cadr x)))

      (acons x)
      (list 'list
            (list 'join
                  (%qq-expand-list (car x))
                  (%qq-expand (cdr x))))

      (list 'quote (list x))))

(mac quasiquote (x)
  (%qq-expand x))

(mac with (params . body)
  `((fn ,(map1 car (pair params)) ,@body)
    ,@(map1 cadr (pair params))))

(mac let (var val . body)
  `(with (,var ,val)
     ,@body))
