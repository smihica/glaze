;; TODO support ring-list

(def caar (x) (car (car x)))

(def cadr (x) (car (cdr x)))

(def cddr (x) (cdr (cdr x)))

(def caddr (x) (car (cddr x)))

(def cdddr (x) (cdr (cddr x)))

(def cadddr (x) (car (cdddr x)))

(def caris (x y)
  (and (acons x)
       (is (car x) y)))

(def carif (lis)
  (if (list? lis)
      (car lis)
      lis))

(mac conswhen (pre arg base)
  (w/uniq (s-arg s-base)
    `(with (,s-arg ,arg)
       (if (,pre ,s-arg)
           (cons ,s-arg ,base)
           ,base))))

(mac consif (pre lis)
  (w/uniq (s-pre)
    `(let ,s-pre ,pre
       (if ,pre
           (cons ,pre ,lis)
           ,lis))))

(def firstn (n lis)
  (def iter (n lis acc)
    (if (<= n 0)
        acc
        (iter (- n 1) (cdr lis) (cons (car lis) acc))))
  (rev (iter n lis nil)))

(def nthcdr (n lis)
  (if (or (<= n 0) (no lis))
      lis
      (nthcdr (- n 1) (cdr lis))))

(def nth (n lis)
  (car (nthcdr n lis)))

(def last (lis)
  (if (no (cdr lis))
      (car lis)
      (last (cdr lis))))

(= alist acons)
(def flat arg
  (def iter (l d)
    (if (and (no (is l nil)) (< 0 d))
        (let i (car l)
          (append
            (if (alist i)
                (iter i (- d 1))
                (list i))
            (iter (cdr l) d)))
        l))
  (iter (car arg) (or (cadr arg) 10000)))


(def rev (lis)
  (def iter (lis acc)
    (if (no lis)
        acc
        (iter (cdr lis) (cons (car lis) acc))))
  (iter lis nil))

;(def rev (x)
;  (def loop (x y)
;    (if (no x)
;        y
;        (let temp (cdr x)
;          (set-cdr! x y)
;          (loop temp x))))
;  (loop x nil)))

(def mappend (f xs)
  (if xs
      (append (f (car xs)) (mappend f (cdr xs)))))

(def intersperse (x lis)
  (def iter (lis)
    (if (cdr lis)
        `(,x ,(car lis) ,@(iter (cdr lis)))
        (list x (car lis))))
  (cdr (iter lis)))

(def split (lis x)
  (def iter (x lis acc)
    (if (or (<= x 0) (no lis))
        (list (rev acc) lis)
        (iter (- x 1) (cdr lis) (cons (car lis) acc))))
  (iter x lis nil))

(def tuples (lis . n)
  (let n (or (car n) 2)
    (let sp (split lis n)
      (if (no (cadr sp))
          (cons (car sp) nil)
          (cons (car sp) (tuples (cadr sp) n))))))

(def range (x y)
  (def iter (z)
    (if (> z y)
        nil
        (cons z (iter (+ 1 z)))))
  (iter x))

(mac n-of (x expr)
  (w/uniq (s_x s_iter)
    `((fn (,s_x)
        (def ,s_iter (,s_x)
          (if (< ,s_x 1)
              nil
              (cons ,expr (,s_iter (- ,s_x 1)))))
        (,s_iter ,s_x)) ,x)))

(def find (f x lis)
  (if lis (if (f x (car lis)) x (find f x (cdr lis)))))

(def rem (test seq)
  (if seq
      (if (is (car seq) test)
          (rem test (cdr seq))
          (cons (car seq) (rem test (cdr seq))))))

(def union (f . liss)
  (def iter (lis acc)
    (if lis
        (if (find f (car lis) acc)
            (iter (cdr lis) acc)
            (iter (cdr lis) (cons (car lis) acc)))
        acc))
  (if liss
      (iter (car liss)
            (apply union f (cdr liss)))))

(def difference (f lis1 lis2)
  (if lis1
      (if (find f (car lis1) lis2)
          (difference f (cdr lis1) lis2)
          (cons (car lis1)
                (difference f (cdr lis1) lis2)))))

(def intersection (f lis1 lis2)
  (if lis1
      (if (find f (car lis1) lis2)
          (cons (car lis1) (intersection f (cdr lis1) lis2))
          (intersection f (cdr lis1) lis2))))
