(mac record (vars val . exps)
  `(apply (fn ,vars ,@exps) ,val))

(mac record-case (target . conds)
  (let target-s (uniq)
    `(let ,target-s ,target
       (if ,@(map1 (fn (c)
                     (let top (car c)
                       (if (or (is top nil) (is top 'else)) c
                           `((is (car ,target-s) ',top)
                             (record ,(cadr c) (cdr ,target-s) ,@(cddr c))))))
                   conds)))))

(mac record-case (target . conds)
  (w/uniq (target-s)
    `(let ,target-s ,target
       (if ,@(flat
               (map1 (fn (c)
                       (let top (car c)
                         (if (or (is top nil) (is top 'else)) c
                             `((is (car ,target-s) ',top)
                               (record ,(cadr c) (cdr ,target-s) ,@(cddr c))))))
                     conds)
               1)))))

(def compile-arc (body binds)
  (disp body binds))